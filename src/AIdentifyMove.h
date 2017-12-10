/*
 * @file AIdentifyMove.h 声明文件, 关联识别位置移动天体
 * @version 0.1
 * @date 2017-12-1
 * @note
 * - 输入单个OT信息
 */

#ifndef AIDENTIFYMOVE_H_
#define AIDENTIFYMOVE_H_

#include <vector>
#include <string>
#include "ADefine.h"

using std::string;

namespace AstroUtil {
///////////////////////////////////////////////////////////////////////////////
class AIdentifyMove {
public:
	AIdentifyMove();
	virtual ~AIdentifyMove();

public:
	/* 数据结构 */
	/*!
	 * @struct 单个瞬变源
	 */
	typedef struct ObservedTransient {
		double x, y;		//< XY坐标
		double ra, dc;	//< 赤经/赤纬, 量纲: 角度
		double mag;		//< 星等
		int type;
	} OT, * PtrOT;
	typedef std::vector<OT> OTVec;

	/*!
	 * @struct 单幅图像中的所有瞬变源
	 */
	typedef struct FrameObservedTransient {
		double secs;		//< 图像对应的当日秒数
		OTVec ots;		//< 图像中瞬变源集合
	} FrmOT, * PtrFrmOT;

protected:
	/* 成员变量 */

public:
	/* 接口 */

protected:
	/* 功能 */
};
///////////////////////////////////////////////////////////////////////////////
} /* namespace AstroUtil */

#endif /* AIDENTIFYMOVE_H_ */
