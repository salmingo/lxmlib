/*
 名称 : lxmlib.cpp
 作者 : 卢晓猛
 版本 : 0.1
 版权 :
 描述 :
 - 归档自开发常用软件
 - 测试自开发常用软件
 */

#include <stdlib.h>
#include <boost/asio.hpp>
#include "GLog.h"
#include "ADefine.h"
#include "WCSTNX.h"

using std::string;
using namespace AstroUtil;

GLog _gLog(stdout);

int main(int argc, char **argv) {
	boost::asio::io_service ios;
	boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
	signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

///////////////////////////////////////////////////////////////////////////////
// 功能测试区
	if (argc < 4) {
		printf("Usage:\n");
		printf("\t lxmlib path_of_acc x y\n");
		return -1;
	}

	WCSTNX tnx;
	if (tnx.LoadText(argv[1])) {
		printf("invalid caliration file\n");
		return -2;
	}
	double x = atof(argv[2]);
	double y = atof(argv[3]);
	double ra, dc;

	tnx.XY2WCS(x, y, ra, dc);
//	_gLog.Write("(x, y) => (ra, dec) : %7.2f  %7.2f  %9.5f  %9.5f", x, y, ra * R2D, dc * R2D);
	printf("(x, y) => (ra, dec) : %7.2f  %7.2f  %9.5f  %9.5f\n", x, y, ra * R2D, dc * R2D);
	tnx.WCS2XY(ra, dc, x, y);
//	_gLog.Write("(ra, dc) => (x, y) : %9.5f  %9.5f  %7.2f  %7.2f", ra * R2D, dc * R2D, x, y);
	printf("(ra, dc) => (x, y) : %9.5f  %9.5f  %7.2f  %7.2f\n", ra * R2D, dc * R2D, x, y);

//////////////////////////////////////////////////////////////////////////////

	ios.run();

	return 0;
}
