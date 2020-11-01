/**
 * @class BuildMatchingShape 创建用于坐标系匹配的模型
 * @version 0.1
 * @date 2020-11-01
 * @author 卢晓猛
 * @note
 * 功能集:
 * - 为图像坐标系建立模型. 亮度项=流量
 * - 为世界坐标系建立模型. 亮度项=星等
 *
 * @note
 * 调用流程:
 * - SetParamPath
 * - SetCoordinateSystem
 * - Prepare
 * - ImportSample
 * - Complete
 * - Build
 * - GetResult
 *
 * @note
 * 依赖库:
 * @li ParamBuildMatchingShape以xml文件形式管理在文件中的参数文件, 依赖库:
 *     boost::property_tree
 */

#ifndef SRC_BUILDMATCHINGSHAPE_H_
#define SRC_BUILDMATCHINGSHAPE_H_

#include <string.h>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
/*!
 * @struct ParamBuildMatchingShape BuildMatchingShape创建模型的约束参数
 */
typedef struct ParamBuildMatchingShape {
	/*!
	 * - 参与构建模型样本数量的最大值
	 * - 有效范围: [10, infinite)
	 * - 缺省值: 10
	 * - 超出范围时采用缺省值
	 */
	int count_sample_max_;
	/*!
	 * - 模型中样本数量的最小值
	 * - 不含中心和定向两点
	 * - 有效范围: [1, infinite)
	 */
	int count_per_shape_min_;
	/*!
	 * - 顶角, 量纲: 角度
	 * - 有效范围: (20, 340)
	 * - 超出该范围时, 由主程序自动判定顶角
	 */
	double apex_angle_;
	/*!
	 * - 样本与中心点的距离, 与坐标系维度长度的比值的最小值
	 * - 有效范围: [0.01, 0.2]
	 * - 缺省值: 0.1
	 * - 超出范围时, 采用缺省值
	 */
	double distance_min_;

protected:
	char pathname[200];	///< 文件路径

public:
	/** 重定义: 操作符 **/
	ParamBuildMatchingShape& operator=(const ParamBuildMatchingShape& other) {
		if (this != &other) memcpy(this, &other, sizeof(ParamBuildMatchingShape));
		return *this;
	}

	bool operator!=(const ParamBuildMatchingShape& other) {
		return (memcmp(this, &other, sizeof(ParamBuildMatchingShape)) == 0);
	}

	/*--------------- 接口 ---------------*/
	/*!
	 * @brief 有效范围检测并返回参数
	 */
	int count_sample_max();
	int count_per_shape_min();
	double apex_angle();
	double distance_min();
	/*!
	 * @brief 加载约束参数
	 * @param filepath  文件路径
	 * @return
	 * 文件访问及参数加载结果.
	 * - 0: 成功
	 * - 1: 文件路径长度超过限制
	 * - 2: 文件不存在或不可访问
	 */
	int Load(const char* filepath);
	/*!
	 * @brief 将约束参数保存至文件
	 * @return
	 * 文件保存结果.
	 * - 0: 成功
	 * - 1: 文件路径为空
	 * - 2: 文件不可写入
	 */
	int Save();
} ParamBMS;

/////////////////////////////////////////////////////////////////////////////
/*!
 * @struct PointBuildMatchingShape 定义: 用于模型匹配的目标坐标
 */
typedef struct PointBuildMatchingShape {
	int id;		///< 编号
	double x, y;///< 坐标
	double z;	///< 亮度

public:
	PointBuildMatchingShape() {
		id = 0;
		x = y = z = 0.0;
	}

	PointBuildMatchingShape(int _id) {
		id = _id;
		x = y = z = 0.0;
	}

	PointBuildMatchingShape(int _id, double _x, double _y) {
		id = _id;
		x  = _x;
		y  = _y;
		z  =  0.0;
	}

	PointBuildMatchingShape(int _id, double _x, double _y, double _z) {
		id = _id;
		x  = _x;
		y  = _y;
		z  = _z;
	}

	PointBuildMatchingShape& operator=(const PointBuildMatchingShape& x) {
		if (this != &x) memcpy(this, &x, sizeof(PointBuildMatchingShape));
		return *this;
	}
} PtBMS;
typedef std::vector<PtBMS> PtBMSVec;

/////////////////////////////////////////////////////////////////////////////
class BuildMatchingShape {
public:
	BuildMatchingShape();
	virtual ~BuildMatchingShape();

public:
	/////////////////////////////////////////////////////////////////////////////
	/*------------------- 数据类型 -------------------*/
	enum {
		COORSYS_MIN,	///< 无效值
		COORSYS_IMAGE,	///< 图像坐标系
		COORSYS_WCS,	///< 世界坐标系
		COORSYS_MAX		///< 无效值
	};

	/*!
	 * @struct Dimension 2维坐标系下的维度长度
	 * @note
	 * - 图像系: 图像的宽度和高度
	 * - 世界系: w==h, 视场角, 量纲: 弧度
	 */
	struct Dimension {
		double w, h;	///< 维度长度
	};

protected:
	/////////////////////////////////////////////////////////////////////////////
	/*------------------- 成员变量 -------------------*/
	ParamBMS param_bak_;///< 约束参数: 初始加载
	ParamBMS param_;	///< 约束参数: 实时
	int coorsys_;		///< 坐标系类型

	bool prepared_;		///< 准备完成标志
	PtBMS ptRef_;		///< 中心点, 仅用于世界系
	PtBMSVec ptSet_;	///< 坐标集合
	Dimension dimension_;	///< 坐标系维度

protected:
	/////////////////////////////////////////////////////////////////////////////
	/*------------------- 功能 -------------------*/

public:
	/////////////////////////////////////////////////////////////////////////////
	/*------------------- 接口 -------------------*/
	/*!
	 * @brief 设置记录约束参数的文件路径
	 * @param filepath  文件路径
	 * @return
	 * 约束参数从文件中的加载结果
	 */
	bool SetParamPath(const char* filepath);
	/*!
	 * @brief 设置坐标系类型
	 * @param coorsys 坐标系类型
	 */
	bool SetCoordinateSystem(int coorsys);
	/*!
	 * @brief 准备导入样本数据
	 * @param ptRef  参考点, 仅适用于世界系, 指定投影中心
	 * @return
	 * 准备工作完成结果.
	 * - 0: 成功
	 * - 1: 坐标系错误
	 * - 2: 无效参考点
	 */
	int Prepare(PtBMS* ptRef = NULL);
	/*!
	 * @brief 导入单个样本
	 * @param pt  样本数据
	 */
	void ImportSample(PtBMS& pt);
	/*!
	 * @brief 检查所有已导入样本的有效性
	 * @return
	 * 样本有效性检查结果
	 */
	bool Complete();
	/*!
	 * @brief 创建模型
	 * @return
	 * 被创建模型的数量
	 */
	int Build();
};

#endif /* SRC_BUILDMATCHINGSHAPE_H_ */
