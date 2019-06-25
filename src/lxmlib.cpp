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
#include "include/sofa.h"

using namespace std;
using namespace AstroUtil;

int main(int argc, char **argv) {
	ATimeSpace ats;
	int iy, im, id, hh, mm;
	double fd, ss;
	double mjd, jd, tai, ut1, era;
	double gmst;

	iy = 2019;
	im = 6;
	id = 22;
	hh = 8;
	mm = 30;
	ss = 0.0;
	fd = (hh + (mm + ss / 60.0) / 60.0) / 24.0;

	printf("from ATimeSpace: \n");
	ats.SetUTC(iy, im, id, fd);
	ats.ModifiedJulianDay(mjd);
	ats.JulianDay(jd);
	ats.TAI(tai);
	ats.UT1(ut1);
	ats.ERA(era);
	printf("MJD = %.11f\n"
			"JD = %f\n"
			"TAI = %.11f\n"
			"UT1 = %.11f\n"
			"ERA = %.11f\n",
			mjd, jd, tai, ut1, era * R2D);
	ats.GMST(gmst);
	ats.H2HMS(gmst * R2D / 15.0, hh, mm, ss);
	printf("gmst = %02d:%02d:%09.6f\n", hh, mm, ss);

	printf("\nfrom SOFA: \n");
	double djm0, djm;
	double tai1, tai2;
	double dut1, ut11, ut12;
	double tt1, tt2;

	iauCal2jd(iy, im, id, &djm0, &djm);
	djm += fd;
	printf("djm0 = %f, djm = %.11f\n", djm0, djm);
	iauUtctai(djm0, djm, &tai1, &tai2);
	printf("tai1 = %f, tai2 = %.11f\n", tai1, tai2);
	dut1 = ats.DeltaUT1(mjd);
	iauUtcut1(djm0, djm, dut1, &ut11, &ut12);
	printf("ut11 = %f, ut12 = %.11f\n", ut11, ut12);
	era = iauEra00(ut11, ut12);
	iauTaitt(tai1, tai2, &tt1, &tt2);
	printf("ERA = %.11f\n", era * R2D);
	gmst = iauGmst06(ut11, ut12, tt1, tt2);
	ats.H2HMS(gmst * R2D / 15.0, hh, mm, ss);
	printf("gmst = %02d:%02d:%09.6f\n", hh, mm, ss);

	return 0;
}
