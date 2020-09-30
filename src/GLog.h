/*!
 * @file GLog.h  类GLog声明文件
 * @author       卢晓猛
 * @description  日志文件访问接口
 * @version      2.0
 * @date         2016年10月28日
 * @version      2.1
 * @date         2020年7月4日
 * - 日志时标由本地时变更为UTC. 原因: 将一个观测夜的日志记录在一个文件中
 * - 当待写盘数据超过1KB时, 立即写盘
 * - 当持续10秒无新日志时, 立即写盘
 * - 增加线程:
 * @note
 * 使用互斥锁管理文件写入操作, 将并行操作转换为串性操作, 避免日志混淆
 */

#ifndef SRC_GLOG_H_
#define SRC_GLOG_H_

#include <stdio.h>
#include <string>
#include <mutex>
#include <chrono>

enum LOG_TYPE {// 日志类型
	LOG_NORMAL,		/// 普通
	LOG_WARN,		/// 警告
	LOG_FAULT,		/// 错误
	LOG_OVER,		/// 完成
	LOG_INTR,		/// 中断
	LOG_IGNR		/// 忽略
};

static const char *LOG_TYPE_STR[] = {
	"",
	"WARN: ",
	"Error: ",
	"OVER ",
	"INTR ",
	"IGNR "
};

class GLog {
public:
	using SystemClock = std::chrono::system_clock;

public:
	GLog(FILE *out = NULL);
	GLog(const char* dirName, const char* fileNamePrefix);
	virtual ~GLog();

public:
	/*!
	 * @brief 记录日志
	 * @param where   日志发生位置
	 * @param type    日志类型
	 * @param format  日志格式
	 */
	void Write(const char *format, ...);
	void Write(LOG_TYPE type, const char *format, ...);
	void Write(const char *where, LOG_TYPE type, const char *format, ...);

protected:
	/*!
	 * @brief 依据时间检查是否需要创建新的日志文件
	 * @return
	 */
	bool valid_file();

protected:
	/* 成员变量 */
	std::mutex	mtx_;		//< 互斥锁
	FILE		*fd_;		//< 文件描述符
	std::string	dirName_;	//< 日志目录
	std::string prefix_;	//< 日志文件名前缀
	int			dayOld_;	//< UTC日期
	int			waitFlush_;	//< 待写入磁盘数据长度
};

#endif /* SRC_GLOG_H_ */
