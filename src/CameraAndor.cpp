/**
 * @class CameraAndor 基于SDK, 实现对Andor CCD的控制接口
 * @version 1.0
 * @date 2020-10-12
 */
#include <unistd.h>
#include "CameraAndor.h"

CameraAndor::CameraAndor() {
	pathxml_ = "/usr/local/etc/andor.xml";
}

CameraAndor::~CameraAndor() {
}

int CameraAndor::CameraNumber() {
	int count(0);
	GetAvailableCameras(&count);
	return count;
}

bool CameraAndor::open_camera(int index) {
	int handler, rslt(0);
	if (index < CameraNumber()
			&& GetCameraHandle(index, &handler) == DRV_SUCCESS
			&& SetCurrentCamera(handler) == DRV_SUCCESS
			&& Initialize((char*)("/usr/local/etc/andor")) == DRV_SUCCESS) {
		sleep(2);	// 等待初始化完成
		rslt |= SetReadMode(4);	// 图像模式
		rslt |= SetAcquisitionMode(1);	// 单帧模式
		SetBaselineClamp(1);
	}
	return rslt == DRV_SUCCESS;
}

void CameraAndor::close_camera() {
	ShutDown();
}

bool CameraAndor::initialize() {
	int rslt(0);
	/* 标准 */
	char model[200];
	rslt |= GetHeadModel(model);
	param_cam_.model = model;
	rslt |= GetDetector(&param_cam_.sensorW, &param_cam_.sensorH);
	rslt |= GetPixelSize(&param_cam_.pixelX, &param_cam_.pixelY);
	/* A/D */
	int nAD, nPort, nRate, nGain, status, h, i, j, k;
	float value;

	/* A/D通道 */
	rslt |= GetNumberADChannels(&nAD);
	for (i = 0; i < nAD; ++i) {
		CameraADChannel item;
		rslt |= GetBitDepth(i, &item.bitdepth);
		item.index = i;
		param_cam_.adc.push_back(item);
	}

	/* 读出端口 */
	rslt |= GetNumberAmp(&nPort);
	for (i = 0; i < nPort; ++i) {
		CameraReadport item;
		rslt |= GetAmpDesc(i, item.name, 40);
		item.index = i;
		param_cam_.readport.push_back(item);
	}

	/* 读出速度 */
	for (i = 0; i < nAD; ++i) {
		for (j = 0; j < nPort; ++j) {
			CameraReadrateSet rateSet;
			rateSet.set_IDs(i, j);
			rslt |= GetNumberHSSpeeds(i, j, &nRate);
			for (k = 0; k < nRate; ++k) {
				CameraReadrate item;
				rslt |= GetHSSpeed(i, j, k, &value);
				item.index = k;
				item.set_rate(value);
				rateSet.readrate.push_back(item);
			}
			param_cam_.readrate.push_back(rateSet);
		}
	}

	/* 增益 */
	rslt |= GetNumberPreAmpGains(&nGain);
	for (i = 0; i < nAD; ++i) {
		rslt |= SetADChannel(i);
		for (j = 0; j < nPort; ++j) {
			rslt |= SetOutputAmplifier(j);
			rslt |= GetNumberHSSpeeds(i, j, &nRate);
			for (k = 0; k < nRate; ++k) {
				rslt |= SetHSSpeed(j, k);
				CameraPreampGainSet gainSet;
				gainSet.set_IDs(i,  j, k);
				for (h = 0; h < nGain; ++h) {
					rslt |= IsPreAmpGainAvailable(i, j, k, h, &status);
					if (!status) continue;
					CameraPreampGain item;
					rslt |= GetPreAmpGain(h, &item.value);
					item.index = h;
					gainSet.preampGain.push_back(item);
				}
				param_cam_.preampGain.push_back(gainSet);
			}
		}
	}

	/* 行转移速度 */
	rslt |= 	GetNumberVSSpeeds(&nRate);
	for (i = 0; i < nRate; ++i) {
		CameraLineshift item;
		rslt |= GetVSSpeed(i, &item.value);
		item.index = i;
		param_cam_.vsrate.push_back(item);
	}

	/* EM */
	AndorCapabilities caps;
	caps.ulSize = sizeof(AndorCapabilities);
	rslt |= GetCapabilities(&caps);
	param_cam_.EM.support = caps.ulCameraType == 1 || caps.ulCameraType == 3;
	param_cam_.EM.low = 0;
	param_cam_.EM.high = 4095;

	/* Cooler */
	param_cam_.cooler.support = caps.ulSetFunctions & AC_SETFUNCTION_TEMPERATURE;
	rslt |= GetTemperatureRange(&param_cam_.cooler.low, &param_cam_.cooler.high);

	/* Shutter */
	param_cam_.shtr.hasMech = caps.ulFeatures & AC_FEATURES_SHUTTER;
	if (param_cam_.shtr.hasMech)
		rslt |= GetShutterMinTimes(&param_cam_.shtr.tmclose, &param_cam_.shtr.tmopen);

	return rslt == DRV_SUCCESS;
}

bool CameraAndor::update_roi(int xbin, int ybin, int xb, int yb, int width, int height) {
	return SetImage(xbin, ybin, xb + 1, xb + width, yb + 1, yb + height) == DRV_SUCCESS;
}

bool CameraAndor::update_env_adchannel(int index) {
	return SetADChannel(index) == DRV_SUCCESS;
}

bool CameraAndor::update_env_readport(int index) {
	return SetOutputAmplifier(index) == DRV_SUCCESS;
}

bool CameraAndor::update_env_readrate(int index) {
	return SetHSSpeed(env_work_.readport.index, index) == DRV_SUCCESS;
}

bool CameraAndor::update_env_preamp_gain(int index) {
	return SetPreAmpGain(index) == DRV_SUCCESS;
}

bool CameraAndor::update_vsrate(int index) {
	return SetVSSpeed(index) == DRV_SUCCESS;
}

void CameraAndor::update_adcoffset(int offset) {
	int state;
	if (GetBaselineClamp(&state) == DRV_SUCCESS) {
		bool rslt(true);
		if (offset == 0 && state == 1) SetBaselineClamp(0);
		else if (offset && state == 0) rslt = SetBaselineClamp(1) == DRV_SUCCESS;
		if (offset && rslt) SetBaselineOffset(offset);
	}
}

bool CameraAndor::update_cooler(double coolerset, bool onoff) {
	int state;
	if (IsCoolerOn(&state) != DRV_SUCCESS) return false;
	if (onoff) {// 制冷
		if (state == 0 && CoolerON() != DRV_SUCCESS) return false;
		return SetTemperature(coolerset) == DRV_SUCCESS;
	}
	// 停止制冷
	if (state == 1) return CoolerOFF() == DRV_SUCCESS;
	return true;
}

bool CameraAndor::sensor_temperature(int &coolerget) {
	int rslt = GetTemperature(&coolerget);
	return !(rslt == DRV_NOT_INITIALIZED || rslt == DRV_ERROR_ACK);
}

bool CameraAndor::update_emmode(int mode) {
	int rslt(0);
	if (0 <= mode && mode <= 3 && env_work_.readport.index == 0) {
		rslt  = SetEMAdvanced(mode == 0 ? 0 : 1);
		rslt |= SetEMGainMode(mode);
	}
	return rslt == DRV_SUCCESS;
}

bool CameraAndor::update_emgain(int &gain) {
	int rslt(0);
	if (env_work_.readport.index == 0) {
		rslt  = SetEMCCDGain(gain);
		rslt |= GetEMCCDGain(&gain);
	}
	return rslt == DRV_SUCCESS;
}

bool CameraAndor::update_shutter(int mode, int tmopen, int tmclose) {
	return SetShutter(1, mode, tmopen, tmclose) == DRV_SUCCESS;
}

bool CameraAndor::start_expose(double expdur) {
	return (SetExposureTime(float(expdur)) == DRV_SUCCESS
			&& StartAcquisition() == DRV_SUCCESS);
}

bool CameraAndor::stop_expose() {
	return AbortAcquisition() == DRV_SUCCESS;
}

int CameraAndor::expose_state() {
	int state, rslt(CAMERA_ERROR);
	GetStatus(&state);
	if (state == DRV_ACQUIRING) rslt = CAMERA_EXPOSE;
	else if (state == DRV_IDLE) rslt = abort_expose_ ? CAMERA_IDLE : CAMERA_IMGRDY;
	return rslt;
}

bool CameraAndor::download_image() {
	int bitdepth = env_work_.adchannel.bitdepth;
	int rslt(0);
	if (bitdepth > 16)     rslt = GetAcquiredData((int*)data_.get(), env_work_.PixelsROI());
	else if (bitdepth > 8) rslt = GetAcquiredData16((uint16_t*)data_.get(), env_work_.PixelsROI());
	return rslt == DRV_SUCCESS;
}
