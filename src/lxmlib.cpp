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
#include "ATimeSpace.h"
#include "ADIProcess.h"

using std::string;
using namespace astro_utility;

GLog _gLog(stdout);

void resolve_ot1(const char *filepath) {
	/*
	 * @note 按相机编号分解数据
	 */
	const int size(200);
	char line[size];
	char *token;
	char seps[] = " \t,\r";
	int year, month, day, hour, minute;
	double second, secs;
	double ra, dc, mag, x, y;
	int cid, ocid(-1);
	int pos, n(0);

	char filename[30];
	FILE *src = fopen(filepath, "rt");
	FILE *dst = NULL;

	while (!feof(src)) {
		if (!fgets(line, size, src)) continue;

		pos = 0;
		token = strtok(line, seps);
		while (token) {
			switch(++pos) {
			case 1:
				year = atoi(token);
				break;
			case 2:
				month = atoi(token);
				break;
			case 3:
				day = atoi(token);
				break;
			case 4:
				hour = atoi(token);
				break;
			case 5:
				minute = atoi(token);
				break;
			case 6:
				second = atof(token);
				break;
			case 9:
				ra = atof(token);
				break;
			case 10:
				dc = atof(token);
				break;
			case 11:
				mag = atof(token);
				if (mag > 99.999) mag = 99.999;
				break;
			case 12:
				x = atof(token);
				break;
			case 13:
				y = atof(token);
				break;
			case 14:
				cid = atoi(token);
				break;
			default:
				break;
			}
			token = strtok(NULL, seps);
		}
		secs = (hour * 60 + minute) * 60 + second;

		if (cid != ocid) {
			ocid = cid;
			if (dst) fclose(dst);

			sprintf(filename, "G%03d.txt", cid);
			dst = fopen(filename, "a+t");

			printf("cid = %d\n", cid);
		}
		fprintf(dst, "%9.3f %7.2f %7.2f %9.5f %9.5f %6.3f\r\n",
				secs, x, y, ra, dc, mag);
	}
	if (dst) fclose(dst);

	fclose(src);
}

int main(int argc, char **argv) {
	boost::asio::io_service ios;
	boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
	signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

///////////////////////////////////////////////////////////////////////////////
// 功能测试区
	ATimeSpace ats;

//////////////////////////////////////////////////////////////////////////////
	ios.run();

	return 0;
}
