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

using std::vector;
using std::string;

GLog _gLog(stdout);

int main(int argc, char **argv) {
	boost::asio::io_service ios;
	boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
	signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

///////////////////////////////////////////////////////////////////////////////
/// 功能测试区

///////////////////////////////////////////////////////////////////////////////

	ios.run();

	return 0;
}
