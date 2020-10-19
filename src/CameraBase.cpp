/**
 * @class CameraBase 天文用相机通用控制接口
 * @version 1.0
 * @date 2020-10-03
 */
#include <boost/bind/bind.hpp>
#include <longnam.h>
#include <fitsio.h>
#include "CameraBase.h"

using namespace boost::placeholders;

CameraBase::CameraBase() {
	byteData_= 0;
	abort_expose_ = false;
}

CameraBase::~CameraBase() {
	Disconnect();
}

/*------------------------ 通用接口 ------------------------*/
const ParamCamera* CameraBase::GetParamCamera() {
	return &param_cam_;
}

const CameraBase::EnvWorking* CameraBase::GetEnvWorking() {
	return &env_work_;
}

const uint8_t* CameraBase::GetData(int &nPixels) {
	nPixels = env_work_.PixelsROI();
	return data_.unique() ? data_.get() : NULL;
}

void CameraBase::RegisterExposeProc(const ExpProcSlot& slot) {
	cb_expproc_.connect(slot);
}

bool CameraBase::IsConnected() {
	return env_work_.connected;
}

bool CameraBase::Connect(int index) {
	if (!IsConnected()
			&& (env_work_.connected = open_camera(index))
			&& !param_cam_.Load(pathxml_)) {
		if (initialize()) param_cam_.Save(pathxml_);
		else {
			close_camera();
			env_work_.state = CAMERA_ERROR;
			env_work_.connected = false;
		}
	}
	if (IsConnected()) {
		UpdateShutterMode(SHTR_AUTO);
		UpdateROI();
		byteData_= env_work_.PixelsROI() * 2;	// 16bit
		data_.reset(new uint8_t[byteData_]);
		env_work_.state = CAMERA_IDLE;

		thrd_idle_.reset(new boost::thread(boost::bind(&CameraBase::thread_idle, this)));
		thrd_expose_.reset(new boost::thread(boost::bind(&CameraBase::thread_expose, this)));
	}

	return IsConnected();
}

void CameraBase::Disconnect() {
	if (IsConnected()) {
		UpdateCooler(0.0, false);	// 停止制冷
		/* 终止曝光并等待曝光结束 */
		AbortExpose();
		while (env_work_.state >= CAMERA_EXPOSE)
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
		interrupt_thread(thrd_expose_);
		/* 等待温度上升至-20以上 */
		while (env_work_.coolerGet < -20) {
			boost::this_thread::sleep_for(boost::chrono::seconds(15));
		}
		interrupt_thread(thrd_idle_);
		/* 断开链接 */
		close_camera();
		env_work_.connected = false;
	}
}

bool CameraBase::UpdateROI(int xbin, int ybin, int xb, int yb, int width, int height) {
	if (xbin <= 0 || xbin > param_cam_.sensorW) return false;
	if (ybin <= 0 || ybin > param_cam_.sensorH) return false;
	if (xb < 0 || xb >= param_cam_.sensorW) return false;
	if (yb < 0 || yb >= param_cam_.sensorH) return false;

	xb -= (xb % xbin);	// 调整起始位置
	yb -= (yb % ybin);
	// 调整区域大小
	if (width <= 0  || (xb + width) > param_cam_.sensorW)  width  = param_cam_.sensorW - xb;
	if (height <= 0 || (yb + height) > param_cam_.sensorH) height = param_cam_.sensorH - yb;
	width  -= (width % xbin);
	height -= (height % ybin);
	if (update_roi(xbin, ybin, xb, yb, width, height)) {
		env_work_.xbin = xbin;
		env_work_.ybin = ybin;
		env_work_.xb   = xb;
		env_work_.yb   = yb;
		env_work_.wroi = width;
		env_work_.hroi = height;
		return true;
	}
	return false;
}

bool CameraBase::Expose(double duration) {
	if (IsConnected()
			&& env_work_.state == CAMERA_IDLE
			&& start_expose(duration)) {
		env_work_.begin_expose(duration);
		abort_expose_ = false;
		cv_exp_.notify_one();
	}
	return env_work_.state == CAMERA_EXPOSE;
}

void CameraBase::AbortExpose() {
	if (env_work_.state >= CAMERA_EXPOSE && stop_expose())
		abort_expose_ = true;
}

void CameraBase::UpdateCooler(int coolerset, bool onoff) {
	CameraLimit &cooler = param_cam_.cooler;
	if (IsConnected() && cooler.support
			&& (!onoff || (cooler.low <= coolerset && coolerset <= cooler.high))
			&& update_cooler(coolerset, onoff)) {
		env_work_.coolerOn = onoff;
		if (onoff) env_work_.coolerSet= coolerset;
	}
}

void CameraBase::UpdateADChannel(int index) {
	if (IsConnected()
			&& env_work_.adchannel.index != index
			&& isvalid_adset(SET_ADCH, index)
			&& update_env_adchannel(index)) {
		env_work_.adchannel = *param_cam_.GetADChannel(index);

		int bitDepth = env_work_.adchannel.bitdepth;
		int bytePixel = bitDepth <= 8 ? 1 : (bitDepth <= 16 ? 2 : 4);
		int byteData = env_work_.PixelsROI() * bytePixel;
		if (byteData != byteData_) {
			byteData_ = byteData;
			data_.reset(new uint8_t[byteData_]);
		}
	}
}

void CameraBase::UpdateReadPort(int index) {
	if (IsConnected()
			&& env_work_.readport.index != index
			&& isvalid_adset(SET_PORT, index)
			&& update_env_readport(index)) {
		env_work_.readport = *param_cam_.GetReadport(index);
	}
}

void CameraBase::UpdateReadRate(int index) {
	if (IsConnected()
			&& env_work_.readrate.index != index
			&& isvalid_adset(SET_RATE, index)
			&& update_env_readrate(index)) {
		env_work_.readrate = *param_cam_.GetReadrate(
				env_work_.adchannel.index, env_work_.readport.index, index);
	}
}

void CameraBase::UpdatePreampGain(int index) {
	if (IsConnected()
			&& env_work_.preampGain.index != index
			&& isvalid_adset(SET_GAIN, index)
			&& update_env_preamp_gain(index)) {
		env_work_.preampGain = *param_cam_.GetPreampGain(
				env_work_.adchannel.index,
				env_work_.readport.index,
				env_work_.readrate.index,
				index);
	}
}

void CameraBase::UpdateVSRate(int index) {
	if (IsConnected() && index != env_work_.vsrate.index) {
		const CameraLineshift *param = param_cam_.GetLineshift(index);
		if (param && update_vsrate(index)) env_work_.vsrate = *param;
	}
}

void CameraBase::UpdateADOffset(int offset) {
	if (IsConnected()) update_adcoffset(offset);
}

void CameraBase::UpdateEMMode(int mode) {
	if (IsConnected()
			&& param_cam_.EM.support
			&& env_work_.EMmode != mode
			&& update_emmode(mode)) {
		env_work_.EMmode = mode;
	}
}

void CameraBase::UpdateEMGain(int gain) {
	if (IsConnected()
			&& env_work_.EMmode >= 0
			&& gain > 0
			&& gain != env_work_.EMgain) {
		update_emgain(gain);
		env_work_.EMgain = gain;
	}
}

void CameraBase::UpdateShutterMode(int mode, int tmopen, int tmclose) {
	if (IsConnected() && param_cam_.shtr.hasMech && mode != env_work_.shtrmode
			&& SHTR_MIN <= mode && mode <= SHTR_MAX) {
		if (tmopen < param_cam_.shtr.tmopen)   tmopen = param_cam_.shtr.tmopen;
		if (tmclose < param_cam_.shtr.tmclose) tmclose = param_cam_.shtr.tmclose;
		if (update_shutter(mode, tmopen, tmclose)) env_work_.shtrmode = mode;
	}
}

bool CameraBase::SampleSaveFITSFile(const char *filepath) {
	string dateobs = to_iso_extended_string (env_work_.dateobs);
	string dateend = to_iso_extended_string (env_work_.dateend);
	fitsfile *iofit;
	int status(0);
	int naxis(2);
	long naxes[] = { param_cam_.sensorW, param_cam_.sensorH };
	int bitDepth = env_work_.adchannel.bitdepth;
	int bitpix = bitDepth <= 8 ? BYTE_IMG : (bitDepth <= 16 ? USHORT_IMG : LONG_IMG);
	int datatyp= bitDepth <= 8 ? TBYTE : (bitDepth <= 16 ? TUSHORT : TLONG);

	fits_create_file(&iofit, filepath, &status);
	/* 数据 */
	fits_create_img(iofit, bitpix, naxis, naxes, &status);
	fits_write_img(iofit, datatyp, 1, env_work_.PixelsROI(), (void*) data_.get(), &status);
	/* 头 */
	fits_write_key_str(iofit, "MODEL",    param_cam_.model.c_str(), "camera model",      &status);
	fits_write_key_flt(iofit, "XPIXSZ",   param_cam_.pixelX, 1, "pixel size in microns", &status);
	fits_write_key_flt(iofit, "YPIXSZ",   param_cam_.pixelY, 1, "pixel size in microns", &status);
	fits_write_key_str(iofit, "READPORT", env_work_.readport.name, "Output Amplifier",   &status);
	fits_write_key_str(iofit, "READRATE", env_work_.readrate.desc, "Readout speed in pixels per second",   &status);
	fits_write_key_flt(iofit, "GAIN",     env_work_.preampGain.value, 1, "Preamp gain in e- per DU", &status);
	fits_write_key_flt(iofit, "VSRATE",   env_work_.vsrate.value, 1, "Line shift speed in microsecs pre line", &status);
	if (param_cam_.EM.support && env_work_.adchannel.index == 0) {
		fits_write_key_log(iofit, "EMGAIN", env_work_.EMgain, "EM gain", &status);
	}
	fits_write_key_log(iofit, "TEMPSET",  env_work_.coolerSet, "Temperature set-point", &status);
	fits_write_key_log(iofit, "TEMPACT",  env_work_.coolerGet, "Tempreature of detector", &status);
	fits_write_key_str(iofit, "DATE-OBS", dateobs.c_str(), "UTC time when start expose",    &status);
	fits_write_key_str(iofit, "DATE-END", dateobs.c_str(), "UTC time when complete expose", &status);
	fits_write_key_dbl(iofit, "EXPTIME",  env_work_.expdur, 6, "expose duration in seconds", &status);

	fits_close_file(iofit, &status);
	return !status;
}

/*------------------------ 底层接口 ------------------------*/
bool CameraBase::isvalid_adset(int type, int index) {
	/* 判定: A/D通道 */
	if (type == SET_ADCH) return param_cam_.GetADChannel(index) != NULL;
	/* 判定: 读出端口 */
	if (type == SET_PORT) return param_cam_.GetReadport(index) != NULL;
	/* 判定: 读出速度 */
	int iAD   = env_work_.adchannel.index;
	int iPort = env_work_.readport.index;
	if (type == SET_RATE) return param_cam_.GetReadrate(iAD, iPort, index) != NULL;
	/* 判定: 增益 */
	int iRate = env_work_.readrate.index;
	if (type == SET_GAIN) return param_cam_.GetPreampGain(iAD, iPort, iRate, index) != NULL;
	/* 无效类型 */
	return false;
}

/*------------------------ 多线程 ------------------------*/
void CameraBase::thread_idle() {
	boost::chrono::seconds t(10);	// 空闲时轮询探测器温度
	int abnormal(0);

	while (1) {
		boost::this_thread::sleep_for(t);
		if (!sensor_temperature(env_work_.coolerGet)) {// 相机异常
			if (++abnormal >= 3) {// 连续异常, 报告错误
				cb_expproc_(0.0, 0.0, CAMERA_ERROR);
			}
		}
		else if (abnormal) abnormal = 0;
	}
}

void CameraBase::thread_expose() {
	boost::chrono::milliseconds t(100);
	boost::mutex mtx;
	MtxLck lck(mtx);
	double left, percent(0.0);
	int &state = env_work_.state;

	while (1) {
		cv_exp_.wait(lck);

		/*
		 * 曝光过程状态变化:
		 * CAMERA_EXPOSE => CAMERA_IMGRDY: 正常结束, 读出图像
		 * CAMERA_EXPOSE => CAMERA_IDLE  : 中止曝光
		 * CAMERA_EXPOSE => CAMERA_ERROR : 错误
		 */
		left = 1000.0;
		/* 等待曝光结束 */
		while ((state = expose_state()) == CAMERA_EXPOSE && left > 0.1) {
			env_work_.expose_process(left, percent);
			if (left > 0.11) {
				cb_expproc_(left, percent, state);
				boost::this_thread::sleep_for(t);
			}
		}
		while ((state = expose_state()) == CAMERA_EXPOSE);
		/* 读出数据 */
		if (state == CAMERA_IMGRDY) {
			env_work_.end_expose();
			if (!download_image()) state = CAMERA_ERROR;
		}
		cb_expproc_(left, percent, state);
		state = CAMERA_IDLE;
	}
}

void CameraBase::interrupt_thread(ThreadPtr& thrd) {
	if (thrd.unique()) {
		thrd->interrupt();
		thrd->join();
		thrd.reset();
	}
}
