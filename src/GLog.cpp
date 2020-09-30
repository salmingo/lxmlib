/*!
 * @file GLog.cpp 类GLog的定义文件
 **/

#include <sys/param.h>
#include <unistd.h>
#include "GLog.h"

GLog::GLog(FILE *out = NULL) {
	dayOld_    = 0;
	waitFlush_ = 0;
	if ((fd_ = out) == NULL) {
		char cwd[MAXPATHLEN];
		dirName_ = getwd(cwd);
		prefix_  = "log_";
	}
}

GLog::GLog(const char* dirName, const char* fileNamePrefix) {
	dayOld_    = 0;
	waitFlush_ = 0;
	fd_        = NULL;
	dirName_   = dirName;
	prefix_    = fileNamePrefix;
}

GLog::~GLog() {
	if (fd_ && fd_ != stdout && fd_ != stderr)
		fclose(fd_);
}

void GLog::Write(const char *format, ...) {

}

void GLog::Write(LOG_TYPE type, const char *format, ...) {

}

void GLog::Write(const char *where, LOG_TYPE type, const char *format, ...) {

}

bool GLog::valid_file() {
	return true;
}
