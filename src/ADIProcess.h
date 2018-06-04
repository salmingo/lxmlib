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
#include <string>
#include <string.h>

using std::vector;
using std::string;

namespace astro_utility {
///////////////////////////////////////////////////////////////////////////////
struct ADIParameter {// 图像处理参数

public:
	ADIParameter &operator=(const ADIParameter &par) {
		if (this != &par) {

		}

		return *this;
	}
};

class ADIProcess {
public:
	ADIProcess();
	virtual ~ADIProcess();

public:
	/* 数据类型 */
	struct ZONE {//< 区域
		int xs, ys;	//< 起始坐标, 原点(0, 0)
		int xe, ye;	//< 结束坐标, 有效区域: [start, end)

	public:
		ZONE() {
			xs = ys = 0;
			xe = ye = 0;
		}
		/*!
		 * @brief 计算区域宽度
		 */
		int width() {
			return (xe - xs);
		}

		/*!
		 * @brief 计算区域高度
		 */
		int height() {
			return (ye - ys);
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

	struct Image {//< 图像基本属性
		bool isvalid;	//< 图像有效性
		double exptime;	//< 曝光时间
		int wimg, himg;	//< 图像宽度与高度
		double *data;	//< 图像数据存储区

	public:
		Image() {
			isvalid = false;
			exptime = 0.0;
			wimg = himg = 0;
			data = NULL;
		}

		virtual ~Image() {
			if (data) free(data);
		}

		/*!
		 * @brief 设置图像宽度与高度
		 */
		bool SetDimension(int w, int h) {
			int nold(wimg * himg), nnew(w * h);
			if (nnew <= 0) return false;
			if (nold != nnew) {
				free(data);
				data = NULL;
			}
			wimg = w;
			himg = h;
			if (!data) data = (double*) malloc(nnew * sizeof(double));
			return (data != NULL);
		}

		/*!
		 * @brief 检查图像尺寸是否一致
		 */
		bool SameDimension(const Image *img) {
			if (!img) return false;
			return (wimg == img->wimg && himg == img->himg);
		}
	};

	typedef vector<string> strvec;

protected:
	/* 成员变量 */
	ADIParameter param_;		//< 图像处理参数
	ZONE zoneLight_;		//< 感光区
	Image imgZero_;		//< 合并后本底
	Image imgDark_;		//< 合并后暗场
	Image imgFlat_;		//< 合并后平场
	Image imgObject_;	//< 待处理图像
	string pathrootCombine_;		//< 参与合并的文件存储目录名
	strvec filenameCombine_;		//< 参与合并的文件名集合

public:
	/* 接口 */
	/*!
	 * @brief 设置感光区坐标
	 * @param zone 区域坐标
	 */
	void SetZoneLight(const ZONE &zone);
	/*!
	 * @brief 设置图像处理参数
	 * @param param 图像处理参数
	 */
	void SetParameter(ADIParameter *param);
	/*!
	 * @brief 设置参与合并的FITS文件存储目录
	 * @param pathroot 目录名称
	 * @return 初始化结果
	 */
	bool InitCombine(const char *pathroot);
	/*!
	 * @brief 添加一个参与合并的FITS文件名
	 * @param filename  文件名
	 */
	void AddFileCombine(const char *filename);
	/*!
	 * @brief 加载合并后本底
	 * @param filepath 文件路径
	 * @return
	 * 文件加载结果
	 */
	bool LoadZero(const char *filepath);
	/*!
	 * @brief 加载合并后暗场
	 * @param filepath 文件路径
	 * @return
	 * 文件加载结果
	 */
	bool LoadDark(const char *filepath);
	/*!
	 * @brief 加载合并后平场
	 * @param filepath 文件路径
	 * @return
	 * 文件加载结果
	 */
	bool LoadFlat(const char *filepath);
	/*!
	 * @brief 加载待处理图像文件
	 * @param filepath 文件路径
	 * @return
	 * 文件加载结果
	 */
	bool LoadImage(const char *filepath);
	/*!
	 * @brief 合并本底
	 * @return 合并结果
	 */
	bool ZeroCombine();
	/*!
	 * @brief 合并暗场
	 * @return 合并结果
	 * @note
	 * 应已合并本底或加载合并后本底
	 */
	bool DarkCombine();
	/*!
	 * @brief 合并平场
	 * @return 合并结果
	 * @note
	 * 至少应已合并本底或加载合并后本底
	 * 已合并暗场或加载合并后暗场
	 */
	bool FlatCombine();
	/*!
	 * @brief 处理已加载待处理图像
	 * @return
	 * 图像处理结果
	 * 0: 成功
	 * 1: 未加载图像文件
	 * 2: 预处理失败
	 */
	int ProcessImage();

protected:
	/* 功能 */
	/*!
	 * @brief 加载图像数据
	 * @param filepath 文件路径
	 * @param image    图像基本内容
	 * @return
	 * 图像加载结果
	 */
	bool load_image(const char *filepath, Image &image);
	/*!
	 * @brief 预处理
	 * @return
	 * 预处理结果
	 * @note
	 * - 检查图像尺寸一致性
	 * - 减本底
	 * - 减暗场
	 * - 除平场
	 */
	bool pre_process();
};
///////////////////////////////////////////////////////////////////////////////
} /* namespace AstroUtil */

#endif /* ADIPROCESS_H_ */
