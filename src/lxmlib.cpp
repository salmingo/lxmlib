/*
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
#include "ADefine.h"
#include "ATimeSpace.h"

using namespace std;
using namespace AstroUtil;

/*
 * 修正儒略日=>贝塞尔历元
 */
double EPB(double mjd) {
	return 1900.0 + (mjd - MJD2K + 36524.68648) / DAYSBY;
}

/*
 * 计算UT2-UT1
 */
double DeltaUT2(double epb) {
	double x = A2PI * epb;
	double dut = 0.022 * sin(x) - 0.012 * cos(x) - 0.006 * sin(2 * x) + 0.007 * cos(2 * x);
	return dut;
}

/*
 * 计算UT1-UTC
 */
double DeltaUT1(double mjd) {
	return (-0.1507 - 0.00063 * (mjd - 58662) - DeltaUT2(EPB(mjd)));
}

int main(int argc, char **argv) {
	ATimeSpace ats;
	double mjd, jd, ydays, ep, jc, fd, ss;
	int iy, im, id, hh, mm;

	iy = 2019;
	im = 6;
	id = 22;
	hh = mm = 0;
	ss = 0.0;
	fd = (hh + (mm + ss / 60.0) / 60.0) / 24.0;

	ats.SetUTC(iy, im, id, fd);
	ats.ModifiedJulianDay(mjd);
	for (int i = 0; i < 10; ++i) {
		ats.MJD2Cal(mjd, iy, im, id, fd);
		ats.H2HMS(fd * 24.0, hh, mm, ss);
		printf("%d %02d %02d %02d %02d %04.1f  %f\n",
				iy, im, id, hh, mm, ss,
				DeltaUT1(mjd));

		mjd += 20.0;
		ats.SetMJD(mjd);
	}

	return 0;
}
