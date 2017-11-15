/*
 名称 : lxmlib.cpp
 作者 : 卢晓猛
 版本 : 0.1
 版权 :
 描述 :
 - 归档自开发常用软件
 - 测试自开发常用软件
 */

#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "GLog.h"
//#include "globaldef.h"
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <dirent.h>
#include "WCSTNX.h"

using std::vector;
using std::string;

GLog _gLog(stdout);

int filter_dir(const struct dirent *ent) {
	if (ent->d_type != DT_REG || ent->d_name[0] == '.') return 0;
	return 1;
}

int main(int argc, char **argv) {
	boost::asio::io_service ios;
	boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
	signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

///////////////////////////////////////////////////////////////////////////////
// 功能测试区
	if (argc < 4) {
//		printf("Usage:\n\t lxmlib path_of_acc path_of_fits\n");
//		printf("Usage:\n\t lxmlib path_of_acc x y\n");
		printf("Usage:\n\t lxmlib path_of_fits x y\n");
		return -1;
	}
//	string fileacc  = argv[1];
	string filefits = argv[1];
	double x = atof(argv[2]);
	double y = atof(argv[3]);
	double ra, dec;
	WCSTNX wcstnx;
	if (wcstnx.LoadImage(filefits.c_str()))
		printf("failed to load WCS from file\n");
	else {
		wcstnx.XY2WCS(x, y, ra, dec);
		printf("%7.2f %7.2f ==> %9.5f %9.5f\n", x, y, ra * R2D, dec * R2D);
	}
//////////////////////////////////////////////////////////////////////////////

	ios.run();

	return 0;
}
