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
	ATimeSpace ats;
	double ra, dec, d_ra, d_dec;
	double stepd(30), stepr;

	ats.SetUTC(2020, 3, 1, 0.0);
	printf ("MJD = %.6f\n", ats.ModifiedJulianDay());
	for (dec = 90.0; dec >= -90.0; dec -= stepd) {
		stepr = stepd / cos(dec * D2R);
		for (ra = 0.0; ra < 360.0; ra += stepr) {
			ats.AnnualAberration(ra * D2R, dec * D2R, d_ra, d_dec);
			printf ("%8.4f %8.4f  %7.2f %7.2f\n", ra, dec, d_ra * R2AS, d_dec * R2AS);
		}
	}
	printf ("\n");

	ats.SetUTC(2020, 9, 1, 0.0);
	printf ("MJD = %.6f\n", ats.ModifiedJulianDay());
	for (dec = 90.0; dec >= -90.0; dec -= stepd) {
		stepr = stepd / cos(dec * D2R);
		for (ra = 0.0; ra < 360.0; ra += stepr) {
			ats.AnnualAberration(ra * D2R, dec * D2R, d_ra, d_dec);
			printf ("%8.4f %8.4f  %7.2f %7.2f\n", ra, dec, d_ra * R2AS, d_dec * R2AS);
		}
	}

	return 0;
}
