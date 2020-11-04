/**
 名称 : lxmlib.cpp
 作者 : 卢晓猛
 版本 : 0.1
 版权 :
 描述 :
 - 归档自开发常用软件
 - 测试自开发常用软件
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADefine.h"
#include "ATimeSpace.h"

using namespace AstroUtil;

int main(int argc, char **argv) {
	if (argc < 4) return -1;

	int year = atoi(argv[1]);
	int month = atoi(argv[2]);
	int day = atoi(argv[3]);
	int hour, minute;
	double second;
	ATimeSpace ats;
	char str[30];

	ats.SetUTC(year, month, day, 0.0);
	double gmst ;
	if (ats.GMST(gmst)) {
		ats.H2HMS(gmst * R2H, hour, minute, second);
		printf ("%02d:%02d:%06.3f\n", hour, minute, second);
	}

	return 0;
}
