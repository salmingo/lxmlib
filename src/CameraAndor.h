/**
 * @class CameraAnndor 基于SDK, 实现对Andor CCD的控制接口
 * @version 1.0
 * @date 2020-10-12
 * @note
 * - 必须执行SetImage(), 否则读出错误
 * - emgain读取错误: 应先设置Readort. 调用顺序错误
 * - gain写入fits错误. SetMCPGain()仅支持iStar, SetPreAmpGain()支持iDus, iXon和Newton
 */

#ifndef SRC_CAMERAANDOR_H_
#define SRC_CAMERAANDOR_H_

#include "CameraBase.h"
#include "atmcdLXd.h"

class CameraAndor: public CameraBase {
public:
	using Pointer = boost::shared_ptr<CameraAndor>;

public:
	CameraAndor();
	~CameraAndor();

public:
	/*!
	 * @brief 查看可用的相机数量
	 */
	int CameraNumber();
	/*!
	 * @brief 创建CameraAndor指针
	 * @return
	 * CameraAndor指针
	 */
	static Pointer Create() {
		return Pointer(new CameraAndor);
	}

protected:
	/*!
	 * @brief 继承类实现与相机的真正连接
	 * @return
	 * 连接结果
	 */
	bool open_camera(int index);
	/*!
	 * @brief 继承类实现真正与相机断开连接
	 */
	void close_camera();
	/*!
	 * @brief 从相机固件中读取相机可配置参数, 并构建ParamCamera对象
	 * @return
	 * 操作结果
	 */
	bool initialize();
	/*!
	 * @brief 设置ROI区域
	 */
	bool update_roi(int xbin, int ybin, int xb, int yb, int width, int height);
	/*!
	 * @brief 改变A/D通道
	 */
	bool update_env_adchannel(int index);
	/*!
	 * @brief 改变读出端口
	 */
	bool update_env_readport(int index);
	/*!
	 * @brief 改变读出速度
	 */
	bool update_env_readrate(int index);
	/*!
	 * @brief 改变前置增益
	 */
	bool update_env_preamp_gain(int index);
	/*!
	 * @brief 改变行转移速度
	 */
	bool update_vsrate(int index);
	/*!
	 * @brief 设置基准偏压
	 */
	void update_adcoffset(int offset);
	/*!
	 * @brief 改变制冷状态和制冷温度
	 */
	bool update_cooler(int coolerset, bool onoff);
	/*!
	 * @brief 采集探测器温度
	 * @param coolerget 探测器温度
	 * @return
	 * 采集结果. 当结果==false时, 相机异常
	 */
	bool sensor_temperature(int &coolerget);
	/*!
	 * @brief 设置EM模式
	 */
	bool update_emmode(int mode);
	/*!
	 * @brief 设置EM增益
	 */
	bool update_emgain(int &gain);
	/*!
	 * @brief 设置快门模式及参数
	 */
	bool update_shutter(int mode, int tmopen, int tmclose);
	/*!
	 * @brief 继承类实现启动真正曝光流程
	 * @param expdur   曝光周期, 量纲: 秒
	 * @return
	 * 曝光启动结果
	 */
	bool start_expose(double expdur);
	/*!
	 * @brief 继承类实现真正中止当前曝光过程
	 */
	bool stop_expose();
	/*!
	 * @brief 查看曝光状态
	 */
	int expose_state();
	/*!
	 * @brief 读出数据
	 * @return
	 * 数据读出结果
	 */
	bool download_image();
};

#endif /* SRC_CAMERAANDOR_H_ */
