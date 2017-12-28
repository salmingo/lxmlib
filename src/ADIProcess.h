/*
 * @file ADIProcess.h 天文数字图像处理声明文件
 * @date 2017-12-28
 * @version 0.1
 * @note
 * - 设置感光区
 * - 合并本底
 * - 合并暗场
 * - 合并平场
 * - 提取图像统计信息
 * - 提取星像信息: 质心, 中心, 流量, FWHM, 椭率
 */

#ifndef ADIPROCESS_H_
#define ADIPROCESS_H_

#include <vector>
#include <string.h>

using std::vector;

namespace AstroUtil {
///////////////////////////////////////////////////////////////////////////////
class ADIProcess {
public:
	ADIProcess();
	virtual ~ADIProcess();

public:
	/* 数据类型 */
	struct ZONE {//< 区域
		int xs, ys;	//< 起始坐标, 原点(0, 0)
		int xe, ye;	//< 结束坐标

	public:
		/*!
		 * @brief 计算区域宽度
		 */
		int width() {
			return (xe - xs + 1);
		}

		/*!
		 * @brief 计算区域高度
		 */
		int height() {
			return (ye - ys + 1);
		}

		ZONE &operator=(const ZONE &x) {
			if (this != &x) {
				memcpy(this, &x, sizeof(ZONE));
			}
			return *this;
		}

		bool operator==(const ZONE &x) {
			return (xs == x.xs && ys == x.ys && xe == x.xe && ye == x.ye);
		}
	};

	struct LIGHT_ZONE {//< 感光区
		int  n;				//< 数量
		vector<ZONE> zones;	//< 感光区坐标
	};

	struct OVERSCAN_ZONE {//< 过扫区: 不感光区
		int  n;				//< 数量
		vector<ZONE> zones;	//< 感光区坐标
	};

	struct Parameter {// 图像处理参数

	public:
		Parameter &operator=(const Parameter &par) {
			if (this != &par) {

			}

			return *this;
		}
	};

protected:
	/* 成员变量 */
	LIGHT_ZONE zoneLight_;		//< 感光区
	OVERSCAN_ZONE zoneOver_;		//< 过扫区
	Parameter param_;			//< 图像处理参数

public:
	/* 接口 */
	/*!
	 * @brief 设置感光区数量
	 * @param n 感光区数量
	 */
	void SetZoneLightNumber(int n);
	/*!
	 * @brief 添加感光区坐标
	 * @param zone 区域坐标
	 */
	void AddZoneLight(const ZONE &zone);
	/*!
	 * @brief 设置过扫区数量
	 * @param n 过扫区数量
	 */
	void SetZoneOverscanNumber(int n);
	/*!
	 * @brief 添加过扫区坐标
	 * @param zone 区域坐标
	 */
	void AddZoneOverscan(const ZONE &zone);
	/*!
	 * @brief 设置图像处理参数
	 * @param param 图像处理参数
	 */
	void SetParameter(Parameter *param);

protected:
	/* 功能 */
};
///////////////////////////////////////////////////////////////////////////////
} /* namespace AstroUtil */

#endif /* ADIPROCESS_H_ */
