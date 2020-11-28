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
#include "AsioUDP.h"

int main(int argc, char **argv) {
	AsioUDP server;
	AsioUDP udp;
	server.Open(3000);
	udp.Open();
	udp.WriteTo("192.168.10.12", 3000, "hello", 5);
	sleep(5);

//	ats.SetUTC(2028, 11, 13, 0.2);
//	ra = 41.0540613;
//	dec = 49.2277489;
//	ats.AnnualAberration(ra * D2R, dec * D2R, d_ra, d_dec);
//	printf ("%.7f  %.7f | %7.2f %7.2f\n", d_ra * R2D, d_dec * R2D, d_ra * R2AS, d_dec * R2AS);

//	double stepd(10), stepr;

//	ats.SetUTC(2020, 11, 20, 0.0);
//	printf ("MJD = %.6f\n", ats.ModifiedJulianDay());
//	for (dec = 90.0; dec >= -90.0; dec -= stepd) {
//		stepr = stepd / cos(dec * D2R);
//		for (ra = 0.0; ra < 360.0; ra += stepr) {
//			ats.AnnualAberration(ra * D2R, dec * D2R, d_ra, d_dec);
//			printf ("%8.4f %8.4f  %7.2f %7.2f\n", ra, dec, d_ra * R2AS, d_dec * R2AS);
//		}
//	}
//	printf ("\n");

//	ats.SetUTC(2020, 11, 21, 0.0);
//	printf ("MJD = %.6f\n", ats.ModifiedJulianDay());
//	for (dec = 90.0; dec >= -90.0; dec -= stepd) {
//		stepr = stepd / cos(dec * D2R);
//		for (ra = 0.0; ra < 360.0; ra += stepr) {
//			ats.AnnualAberration(ra * D2R, dec * D2R, d_ra, d_dec);
//			printf ("%8.4f %8.4f  %7.2f %7.2f\n", ra, dec, d_ra * R2AS, d_dec * R2AS);
//		}
//	}

	return 0;
}
