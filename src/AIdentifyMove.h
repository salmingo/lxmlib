/*
 * @file AIdentifyMove.h 声明文件, 关联识别位置移动天体
 * @version 0.1
 * @date 2017-12-1
 * @note
 */

#ifndef AIDENTIFYMOVE_H_
#define AIDENTIFYMOVE_H_

#include "ADefine.h"

namespace AstroUtil {
///////////////////////////////////////////////////////////////////////////////
class AIdentifyMove {
public:
	AIdentifyMove();
	virtual ~AIdentifyMove();

public:
	/* 数据结构 */
	/*!
	 * @class 帧目标
	 */
	typedef struct frame_objects {
		int  n;		//< 目标数量
		PT3F *pts;	//< 目标位置集合
	} * ptr_frmobj;

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
