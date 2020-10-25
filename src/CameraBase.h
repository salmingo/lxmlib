/**
 * @class CameraBase 天文用相机通用控制接口
 * @version 1.0
 * @date 2020-10-03
 * @note
 * - 相机的工作参数以文件形式存储在/usr/local/etc目录下
 * - 初次连接相机时, 从相机固件中遍历采集工作参数, 之后使用时从文件中读取
 */

#ifndef CAMERABASE_H_
#define CAMERABASE_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include <string>
#include <vector>
#include "AstroDeviceDef.h"
#include "ParamCamera.h"

using std::string;
using std::vector;
using namespace boost::posix_time;

class CameraBase {
public:
	/* 数据类型 */
	using ThreadPtr = boost::shared_ptr<boost::thread>;
	using MtxLck    = boost::unique_lock<boost::mutex>;
	/*!
	 * @brief 曝光进度回调函数
	 * @param <1> 曝光剩余时间, 秒
	 * @param <2> 曝光进度, 百分比
	 * @param <3> 工作状态
	 */
	using ExposeProcess = boost::signals2::signal<void (double, double, int)>;
	using ExpProcSlot   = ExposeProcess::slot_type;

public:
	enum {// 相机工作状态
		CAMERA_ERROR,	// 错误
		CAMERA_IDLE,	// 空闲
		CAMERA_EXPOSE,	// 曝光过程中
		CAMERA_IMGRDY	// 已完成曝光, 可以读出数据进入内存
	};

	enum {// A/D转换设置参数类型
		SET_ADCH = 1,	// A/D通道
		SET_PORT,	// 读出端口
		SET_RATE,	// 读出速度
		SET_GAIN	// 增益
	};

	enum {// 快门模式
		SHTR_MIN  = 0,
		SHTR_AUTO = 0,	// 自动
		SHTR_OPEN,	// 常开
		SHTR_CLOSE,	// 常关
		SHTR_MAX = SHTR_CLOSE
	};

	/*!
	 * @struct EnvWorking 相机工作环境
	 */
	struct EnvWorking {
		bool connected;	//< 连接标志
		/* A/D参数 */
		CameraADChannel  adchannel;
		CameraReadport   readport;
		CameraReadrate   readrate;
		CameraPreampGain preampGain;
		CameraLineshift  vsrate;
		/* EM模式 */
		int EMmode;
		int EMgain;
		/* 制冷 */
		bool coolerOn;
		int coolerSet, coolerGet;
		/* ROI */
		int xbin, ybin;	//< 合并因子
		int xb, yb;		//< 起始位置.  ROI区在原始图像中的位置
		int wroi, hroi;	//< 宽度/高度. ROI区在原始图像中的尺度
		/* 曝光参数 */
		int state;		//< 工作状态
		int errcode;	//< 错误代码
		int shtrmode;	//< 快门模式
		double expdur;	//< 曝光时间
		ptime dateobs;	//< 曝光起始时间对应的日期
		ptime dateend;	//< 曝光结束时间对应的时间

	public:
		EnvWorking() {
			connected = false;
			EMmode = EMgain = -1;
			coolerOn = false;
			coolerSet = coolerGet = 0;
			xbin = ybin = 0;
			xb = yb = wroi = hroi = 0;
			state = CAMERA_ERROR;
			errcode = 0;
			shtrmode= 0;
			expdur = 0.0;
		}

		void begin_expose(double _expdur) {
			dateobs = microsec_clock::universal_time();
			expdur = _expdur;
			state  = CAMERA_EXPOSE;
		}

		void end_expose() {
			dateend = microsec_clock::universal_time();
			state = CAMERA_IMGRDY;
		}

		/*!
		 * @brief 检查曝光进度
		 * @param left    剩余时间
		 * @param percent 曝光进度
		 */
		void expose_process(double& left, double& percent) {
			if (state == CAMERA_EXPOSE) {
				ptime now = microsec_clock::universal_time();
				time_duration elps = now - dateobs;
				double micsec = elps.total_microseconds() * 1.0E-6;
				if ((left = expdur - micsec) < 0.0 || expdur < 1.0E-6) {
					left = 0.0;
					percent = 100.0001;
				}
				else percent = micsec * 100.0001 / expdur;
			}
		}

		/*!
		 * @brief 计算ROI区像素数
		 */
		int PixelsROI() {
			return (wroi * hroi / xbin / ybin);
		}
	};

protected:
	/* 成员变量 */
	string pathxml_;	//< 相机可配置参数文件路径
	ParamCamera param_cam_;		//< 相机可配置参数
	EnvWorking env_work_;		//< 相机工作环境
	boost::shared_array<uint8_t> data_;	//< 图像数据存储区
	int byteData_;	//< 图形数据存储区大小
	/* 工作状态 */
	ExposeProcess cb_expproc_;	//< 回调函数: 曝光进度
	boost::condition_variable cv_exp_;	//< 条件变量: 曝光状态发生变化
	ThreadPtr thrd_idle_;	//< 线程: 空闲, 监测探测器温度, 及相机异常
	ThreadPtr thrd_expose_;	//< 线程: 监测曝光进度和结果
	bool abort_expose_;	//< 中断曝光

public:
	CameraBase();
	virtual ~CameraBase();

public:
	/*!
	 * @brief 查看可用的相机数量
	 */
	virtual int CameraNumber() = 0;
	/*!
	 * @brief 查看相机可配置参数
	 * @return
	 * 相机可配置参数
	 */
	const ParamCamera* GetParamCamera();
	/*!
	 * @brief 查看相机工作环境
	 * @return
	 * 相机工作环境
	 */
	const EnvWorking* GetEnvWorking();
	/*!
	 * @brief 访问数据存储区
	 * @param nPixels 数据存储区像素数
	 * @return
	 * 数据存储区地址
	 */
	const uint8_t* GetData(int &nPixels);
	/*!
	 * @brief 注册曝光进度回调函数
	 * @param slot 插槽函数
	 */
	void RegisterExposeProc(const ExpProcSlot& slot);
	/*!
	 * @brief 相机连接标志
	 * @return
	 * 是否已经建立与相机连接标志
	 */
	bool IsConnected();
	/*!
	 * @brief 尝试连接相机
	 * @param index 相机索引
	 * @return
	 * 相机连接结果
	 * @note
	 * 当连接多台相机时, 索引指定相机的索引
	 */
	bool Connect(int index = 0);
	/*!
	 * @brief 断开与相机的连接
	 */
	void Disconnect();
	/*!
	 * @brief 设置ROI区域
	 * @param xbin     X合并因子
	 * @param ybin     Y合并因子
	 * @param xb       X起始位置, 起始索引: 0
	 * @param yb       Y起始位置, 起始索引: 0
	 * @param width    X方向ROI长度, <=0 代表从xb至sensorW
	 * @param height   Y方向ROI长度, <=0 代表从yb至sensorH
	 * @return
	 * 操作结果
	 * @note
	 * 约束:
	 * - xb % xbin == 0
	 * - yb % ybin == 0
	 * - width % xbin == 0
	 * - height % ybin == 0
	 */
	bool UpdateROI(int xbin = 1, int ybin = 1, int xb = 0, int yb = 0, int width = 0, int height = 0);
	/*!
	 * @brief 尝试启动曝光流程
	 * @param duration  曝光周期, 量纲: 秒
	 * @return
	 * 曝光启动结果
	 */
	bool Expose(double duration);
	/*!
	 * @brief 中止当前曝光过程
	 */
	void AbortExpose();
	/*!
	 * @brief 设置制冷器工作模式及制冷温度
	 * @param coolerset  期望温度, 量纲: 摄氏度
	 * @param onoff      制冷器开关
	 */
	void UpdateCooler(int coolerset = -20, bool onoff = false);
	/*!
	 * @brief 设置A/D通道
	 * @param index A/D通道档位
	 */
	void UpdateADChannel(int index);
	/*!
	 * @brief 设置读出端口
	 * @param index 读出端口档位
	 */
	void UpdateReadPort(int index);
	/*!
	 * @brief 设置读出速度
	 * @param index 读出速度档位
	 */
	void UpdateReadRate(int index);
	/*!
	 * @brief 设置增益
	 * @param index 增益档位
	 */
	void UpdatePreampGain(int index);
	/*!
	 * @brief 设置行转移速度
	 * @param index 速度档位
	 */
	void UpdateVSRate(int index);
	/*!
	 * @brief 设置本底基准值
	 * @param offset 基准值
	 * @note
	 * offset == 0时禁用该功能
	 */
	void UpdateADOffset(int offset);
	/*!
	 * @brief 设置EM模式
	 * @param mode EM模式
	 */
	void UpdateEMMode(int mode = 0);
	/*!
	 * @brief 设置EM增益
	 * @param gain EM增益
	 */
	void UpdateEMGain(int gain);
	/*!
	 * @brief 设置快门工作模式及开关时间
	 * @param mode    模式
	 * @param tmopen  打开时间, 量纲: 毫秒
	 * @param tmclose 关闭时间, 量纲: 毫秒
	 * @note
	 * 当tmopen==0或tmclose==0时, 使用系统默认值
	 */
	void UpdateShutterMode(int mode, int tmopen = 0, int tmclose = 0);

public:
	/*!
	 * @brief 将曝光结果存储为FITS文件
	 * @param filepath 文件路径
	 * @return
	 * 文件存储结果
	 */
	bool SampleSaveFITSFile(const char *filepath);

protected:
	/*!
	 * @brief 检查A/D设置参数的有效性
	 * @param type  A/D设置参数类型
	 * @param index 参数档位
	 * @return
	 * 有效性判定结果
	 */
	bool isvalid_adset(int type, int index);

protected:
	/*!
	 * @brief 继承类实现与相机的真正连接
	 * @return
	 * 连接结果
	 */
	virtual bool open_camera(int index) = 0;
	/*!
	 * @brief 继承类实现真正与相机断开连接
	 */
	virtual void close_camera() = 0;
	/*!
	 * @brief 从相机固件中读取相机可配置参数, 并构建ParamCamera对象
	 * @return
	 * 操作结果
	 */
	virtual bool initialize() = 0;
	/*!
	 * @brief 设置ROI区域
	 */
	virtual bool update_roi(int xbin, int ybin, int xb, int yb, int width, int height) = 0;
	/*!
	 * @brief 改变A/D通道
	 */
	virtual bool update_env_adchannel(int index) = 0;
	/*!
	 * @brief 改变读出端口
	 */
	virtual bool update_env_readport(int index) = 0;
	/*!
	 * @brief 改变读出速度
	 */
	virtual bool update_env_readrate(int index) = 0;
	/*!
	 * @brief 改变前置增益
	 */
	virtual bool update_env_preamp_gain(int index) = 0;
	/*!
	 * @brief 改变行转移速度
	 */
	virtual bool update_vsrate(int index) = 0;
	/*!
	 * @brief 设置基准偏压
	 */
	virtual void update_adcoffset(int offset) = 0;
	/*!
	 * @brief 改变制冷状态和制冷温度
	 */
	virtual bool update_cooler(int coolerset, bool onoff) = 0;
	/*!
	 * @brief 采集探测器温度
	 * @param coolerget 探测器温度
	 * @return
	 * 采集结果. 当结果==false时, 相机异常
	 */
	virtual bool sensor_temperature(int &coolerget) = 0;
	/*!
	 * @brief 设置EM模式
	 */
	virtual bool update_emmode(int mode) = 0;
	/*!
	 * @brief 设置EM增益
	 */
	virtual bool update_emgain(int &gain) = 0;
	/*!
	 * @brief 设置快门模式及参数
	 */
	virtual bool update_shutter(int mode, int tmopen, int tmclose) = 0;
	/*!
	 * @brief 继承类实现启动真正曝光流程
	 * @param expdur   曝光周期, 量纲: 秒
	 * @return
	 * 曝光启动结果
	 */
	virtual bool start_expose(double expdur) = 0;
	/*!
	 * @brief 继承类实现真正中止当前曝光过程
	 */
	virtual bool stop_expose() = 0;
	/*!
	 * @brief 查看曝光状态
	 */
	virtual int expose_state() = 0;
	/*!
	 * @brief 读出数据
	 * @return
	 * 数据读出结果
	 */
	virtual bool download_image() = 0;

protected:
	/*!
	 * @brief 线程: 无曝光时监测相机温度, 并依据监测结果判定相机是否异常
	 */
	void thread_idle();
	/*!
	 * @brief 线程: 监测曝光过程
	 */
	void thread_expose();
	/*!
	 * @brief 中断线程
	 * @param thrd 线程指针
	 */
	void interrupt_thread(ThreadPtr& thrd);
};

#endif /* CAMERABASE_H_ */
