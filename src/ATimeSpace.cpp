/**
 * @file ATimeSpace.cpp
 * @brief 天文时空转换函数接口
 * @date 2019-06-21
 * @version 1.0
 * @author 卢晓猛
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "ADefine.h"
#include "ATimeSpace.h"

namespace AstroUtil {
/////////////////////////////////////////////////////////////////////////////
double ATimeSpace::coef_aab_[36][6] = {
	{   0,  -25,   0,    0,     0,    0 },
	{   0,    0,   0,    0, -2559,    0 },
	{ 715,    0,   6, -657,   -15, -282 },
	{ 715,    0,   0, -656,     0, -285 },
	{   0,    0,   0,    0,   -94, -193 },
	{ 159,    0,   2, -147,    -6,  -61 },
	{   0,    0,   0,   26,     0,  -59 },
	{  39,    0,   0,  -36,     0,  -16 },
	{  33,  -10,  -9,  -30,    -5,  -13 },
	{  31,    1,   1,  -28,     0,  -12 },
	{   8,  -28,  25,    8,    11,    3 },
	{   8,  -28, -25,   -8,   -11,   -3 },
	{  21,    0,   0,  -19,     0,   -8 },
	{ -19,    0,   0,   17,     0,    8 },
	{  17,    0,   0,  -16,     0,   -7 },
	{  16,    0,   0,   15,     1,    7 },
	{  16,    0,   1,  -15,    -3,   -6 },
	{  11,   -1,  -1,  -10,    -1,   -5 },
	{   0,  -11, -10,    0,    -4,    0 },
	{ -11,   -2,  -2,    9,    -1,    4 },
	{  -7,   -8,  -8,    6,    -3,    3 },
	{ -10,    0,   0,    9,     0,    4 },
	{  -9,    0,   0,   -9,     0,   -4 },
	{  -9,    0,   0,   -8,     0,   -4 },
	{   0,   -9,  -8,    0,    -3,    0 },
	{   0,   -9,   8,    0,     3,    0 },
	{   8,    0,   0,   -8,     0,   -3 },
	{   8,    0,   0,   -7,     0,   -3 },
	{  -4,   -7,  -6,    4,    -3,    2 },
	{  -4,   -7,   6,   -4,     3,   -2 },
	{  -6,   -5,  -4,    5,    -2,    2 },
	{  -1,   -1,  -2,   -7,     1,   -4 },
	{   4,   -6,  -5,   -4,    -2,   -2 },
	{   0,   -7,  -6,    0,    -3,    0 },
	{   5,   -5,  -4,   -5,    -2,   -2 },
	{   5,    0,   0,   -5,     0,   -2 }
};

/*--------------------------------------------------------------------------*/
ATimeSpace::ATimeSpace() {
	lon_ = lat_ = alt_ = 0.0;
	dut1_ = 0.0;
}

ATimeSpace::~ATimeSpace() {
}

/////////////////////////////////////////////////////////////////////////////
/* 设置常规参数 */
void ATimeSpace::SetSite(double lon, double lat, double alt) {
	lon_ = lon * D2R;
	lat_ = lat * D2R;
	alt_ = alt;
}

void ATimeSpace::SetUTC(int iy, int im, int id, double fd) {
	const int IYMIN = -4800;
	if (iy > IYMIN && im >= 1 && im <= 12) {
		memset(valid_, 0, sizeof(bool) * NDX_MAX);
		memset(values_, 0, sizeof(double) * NDX_MAX);
		dut1_ = 0.0;
		values_[NDX_MJD] = cal2mjd(iy, im, id, fd);
		valid_[NDX_MJD] = true;
	}
}

void ATimeSpace::SetMJD(double mjd) {
	memset(valid_, 0, sizeof(bool) * NDX_MAX);
	memset(values_, 0, sizeof(double) * NDX_MAX);
	dut1_ = 0.0;
	values_[NDX_MJD] = mjd;
	valid_[NDX_MJD] = true;
}

void ATimeSpace::SetJD(double jd) {
	SetMJD(jd - MJD0);
}

void ATimeSpace::SetEpoch(double ep) {
	SetMJD((ep - 2000.0) * DAYSJY + MJD2K);
}

void ATimeSpace::SetYDays(int iy, double ydays) {
	int im, id;
	int days = int(ydays);
	ydays2ymd(iy, days, im, id);
	SetUTC(iy, im, id, ydays - days);
}

void ATimeSpace::SetDeltaUT1(double dut) {
	dut1_ = dut;
}
/////////////////////////////////////////////////////////////////////////////
/* 格式转换 */
void ATimeSpace::H2HMS(double hour, int&hh, int&mm, double&ss) {
	hh = int(hour);
	hour = (hour - hh) * 60.01;
	mm = int(hour);
	ss = (hour - mm) * 60.01;
}

void ATimeSpace::D2DMS(double deg, int&dd, int&mm, double&ss, int&sign) {
	sign = deg >= 0.0 ? 1 : -1;
	if (sign == -1)
		deg = -deg;
	dd = int(deg);
	deg = (deg - dd) * 60.01;
	mm = int(deg);
	ss = (deg - mm) * 60.01;
}

bool ATimeSpace::hdresolve(const char* str, double& val) {
	int n = int(strlen(str));
	int i, j(-1);
	int part(0); // 0: 时/度; 1: (角)分; 2: (角)秒
	int dotcnt(0); // 小数点数量
	int hd(0), mm(0);
	double ss(0.0);
	char ch, tmp[20];

	// 遍历解析字符串
	for (i = 0; i < n; ++i) {
		if (isnumber(ch = str[i])) {
			if (part < 2 && j == 1 && !dotcnt) {
				tmp[++j] = 0;
				if (part == 0)
					hd = atoi(tmp);
				else
					mm = atoi(tmp);
				++part;
				j = -1;
			}
			tmp[++j] = ch;
		} else if (ch == ':' || ch == ' ') {
			if (part == 2 || dotcnt)
				return false;
			tmp[++j] = 0;
			if (part == 0)
				hd = atoi(tmp);
			else
				mm = atoi(tmp);
			++part;
			j = -1;
		} else if (ch == '.') {
			if (++dotcnt > 1)
				return false;
			tmp[++j] = ch;
		} else
			return false;
	}

	tmp[++j] = 0;
	if (part == 0)
		val = atof(tmp);
	else if (part == 1)
		val = hd + atof(tmp) / 60.0;
	else
		val = hd + (mm + ss / 60.0) / 60.0;
	return true;
}

bool ATimeSpace::Str2H(const char* str, double & hour) {
	if (!(str && hdresolve(str, hour)))
		return false;
	return (0.0 <= hour && hour < 24.0);
}

bool ATimeSpace::Str2D(const char* str, double & deg) {
	if (!(str && hdresolve((str[0] == '+' || str[0] == '-') ? str + 1 : str, deg)))
		return false;

	if (str[0] == '-')
		deg = -deg;
	return (-90.0 <= deg && deg <= 90.0);
}
/////////////////////////////////////////////////////////////////////////////
/* 日历/历书 */
bool ATimeSpace::leap_year(int iy) {
	return (!(iy % 400) || (!(iy % 4) && (iy % 100)));
}

double ATimeSpace::cal2mjd(int iy, int im, int id, double fd) {
	double mjd;
	if (im <= 2) {
		--iy;
		im += 12;
	}
	mjd = int(365.25 * iy) + int(30.6001 * (im + 1)) + id - int(iy / 100)
			+ int(iy / 400) - 679004 + fd;

	return mjd;
}

void ATimeSpace::ydays2ymd(int iy, int ydays, int &im, int &id) {
	int k = leap_year(iy) ? 1 : 2;
	if (ydays <= 31)
		im = 1;
	else
		im = int(9.0 * (k + ydays) / 275.0 + 0.98);
	id = ydays - int(275 * im / 9) + k * int((im + 9) / 12) + 30;
}

int ATimeSpace::ymd2ydas(int iy, int im, int id) {
	int k = leap_year(iy) ? 1 : 2;
	return (int(275 * im / 9) - k * int((im + 9) / 12) + id - 30);
}

void ATimeSpace::MJD2Cal(double mjd, int &iy, int &im, int &id, double &fd) {
	double jd;
	int A, B, C, D, X, Y, Z;

	jd = mjd + MJD0;
	fd = mjd - int(mjd);
	Z = int(jd + 0.5);
	if (Z < 2299161)
		A = Z;
	else {
		X = int((Z - 1867216.25) / 36524.25);
		A = Z + 1 + X - int(X / 4);
	}
	B = A + 1524;
	C = int((B - 122.1) / 365.25);
	D = int(365.25 * C);
	Y = int((B - D) / 30.6001);
	id = B - D - int(30.6001 * Y);
	im = Y < 14 ? Y - 1 : Y - 13;
	iy = im > 2 ? C - 4716 : C - 4715;
}

void ATimeSpace::GetYDays(double mjd, int &iy, double &ydays) {
	int im, id;
	double fd;

	MJD2Cal(mjd, iy, im, id, fd);
	ydays = ymd2ydas(iy, im, id) + fd;
}

double ATimeSpace::ModifiedJulianDay() {
	return values_[NDX_MJD];
}

double ATimeSpace::JulianDay() {
	return values_[NDX_MJD] + MJD0;
}

double ATimeSpace::JulianDay(double mjd) {
	return (mjd + MJD0);
}

double ATimeSpace::JulianCentury() {
	return (values_[NDX_MJD] - MJD2K) / DAYSJC;
}

double ATimeSpace::JulianCentury(double mjd) {
	return (mjd - MJD2K) / DAYSJC;
}

double ATimeSpace::Epoch() {
	return (values_[NDX_MJD] - MJD2K) / DAYSJY + 2000.0;
}

double ATimeSpace::Epoch(double mjd) {
	return (mjd - MJD2K) / DAYSJY + 2000.0;
}

double ATimeSpace::EpochBessel(double mjd) {
	// 36524.68648 = B1900-J2000的天数
	return 1900.0 + (mjd - MJD2K + 36524.68648) / DAYSBY;
}

/////////////////////////////////////////////////////////////////////////////
/*---------------- 时间系统 ----------------*/
double ATimeSpace::DeltaAT(double mjd) {
	/*
	 * 当闰秒更新时, 需更改参数:
	 * - IYV变更为最后更新时间对应的年
	 * - changes[]添加更新数据
	 */
//	const int IYV = 2017;
	/*
	 * 1972年前, 使用时间漂移量计算闰秒
	 */
	static const double drift[][2] = {
		{ 37300.0, 0.0012960 },
		{ 37300.0, 0.0012960 },
		{ 37300.0, 0.0012960 },
		{ 37665.0, 0.0011232 },
		{ 37665.0, 0.0011232 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 38761.0, 0.0012960 },
		{ 39126.0, 0.0025920 },
		{ 39126.0, 0.0025920 }
	};
	const int NERA1 = (int) (sizeof drift / sizeof (double) / 2);

	/* Dates and Delta(AT)s */
	static const struct {
		int iyear, month;
		double delat;
	} changes[] = {
		{ 1960,  1,  1.4178180 },
		{ 1961,  1,  1.4228180 },
		{ 1961,  8,  1.3728180 },
		{ 1962,  1,  1.8458580 },
		{ 1963, 11,  1.9458580 },
		{ 1964,  1,  3.2401300 },
		{ 1964,  4,  3.3401300 },
		{ 1964,  9,  3.4401300 },
		{ 1965,  1,  3.5401300 },
		{ 1965,  3,  3.6401300 },
		{ 1965,  7,  3.7401300 },
		{ 1965,  9,  3.8401300 },
		{ 1966,  1,  4.3131700 },
		{ 1968,  2,  4.2131700 },
		{ 1972,  1, 10.0       },
		{ 1972,  7, 11.0       },
		{ 1973,  1, 12.0       },
		{ 1974,  1, 13.0       },
		{ 1975,  1, 14.0       },
		{ 1976,  1, 15.0       },
		{ 1977,  1, 16.0       },
		{ 1978,  1, 17.0       },
		{ 1979,  1, 18.0       },
		{ 1980,  1, 19.0       },
		{ 1981,  7, 20.0       },
		{ 1982,  7, 21.0       },
		{ 1983,  7, 22.0       },
		{ 1985,  7, 23.0       },
		{ 1988,  1, 24.0       },
		{ 1990,  1, 25.0       },
		{ 1991,  1, 26.0       },
		{ 1992,  7, 27.0       },
		{ 1993,  7, 28.0       },
		{ 1994,  7, 29.0       },
		{ 1996,  1, 30.0       },
		{ 1997,  7, 31.0       },
		{ 1999,  1, 32.0       },
		{ 2006,  1, 33.0       },
		{ 2009,  1, 34.0       },
		{ 2012,  7, 35.0       },
		{ 2015,  7, 36.0       },
		{ 2017,  1, 37.0       }
	};
	const int NDAT = (int) (sizeof changes / sizeof changes[0]);
	int iy, im, id;
	double fd;
	double dat;

	MJD2Cal(mjd, iy, im, id, fd);
	if (iy < changes[0].iyear) return false;

	int i, m;
	m = 12 * iy + im;
	for (i = NDAT-1; i >=0; --i) {
		if (m >= (12 * changes[i].iyear + changes[i].month)) break;
	}
	dat = changes[i].delat;
	if (i < NERA1) dat += (mjd + fd - drift[i][0]) * drift[i][1];

	return dat;
}

double ATimeSpace::DeltaUT2(double epb) {
	double t = A2PI * epb;
	double dut = 0.022 * sin(t) - 0.012 * cos(t) - 0.006 * sin(2 * t) + 0.007 * cos(2 * t);
	return dut;
}

// 计算与UTC对应的TAI的修正儒略日
double ATimeSpace::TAI() {
	if (valid_[NDX_TAI]) {
		int mjd0;
		double mjd, fd;
		double dat0, dat12, dat24;
		double dlod, dleap;

		mjd = values_[NDX_MJD];
		mjd0 = int(mjd);
		fd = mjd - mjd0;
		dat0  = DeltaAT(mjd0);
		dat12 = DeltaAT(mjd0 + 0.5);
		dat24 = DeltaAT(mjd0 + 1.0);
		dlod = 2.0 * (dat12 - dat0);
		dleap = dat24 - (dat0 + dlod);
		fd *= (DAYSEC + dleap) / DAYSEC;
		fd *= (DAYSEC + dlod) / DAYSEC;

		values_[NDX_TAI] = mjd0 + fd + dat0 / DAYSEC;
		valid_[NDX_TAI] = true;
	}
	return values_[NDX_TAI];
}

// 计算与UTC对应的UT1的修正儒略日
double ATimeSpace::UT1() {
	if (!valid_[NDX_UT1]) {
		double mjd = values_[NDX_MJD];
		double tai = TAI();
		double dat = DeltaAT(int(mjd));

		values_[NDX_UT1] = tai + (dut1_ - dat) / DAYSEC;
		valid_[NDX_UT1] = true;
	}
	return values_[NDX_UT1];
}

double ATimeSpace::TT() {
	if (!valid_[NDX_TT]) {
		values_[NDX_TT] = TAI() + TTMTAI / DAYSEC;
		valid_[NDX_TT] = true;
	}
	return values_[NDX_TT];
}

double ATimeSpace::GMST() {
	if (!valid_[NDX_GMST]) {
		double tt = TT();
		double era = ERA();
		double t = (tt - MJD2K) / DAYSJC;
		double gmst = era +
				(   0.014506    +
				(4612.156534    +
				(   1.3915817   +
				(  -0.00000044  +
				(  -0.000029956 +
				(  -0.0000000368)
				* t) * t) * t) * t) * t) * AS2R;
		gmst = crcmod(gmst);
		values_[NDX_GMST] = gmst;
		valid_[NDX_GMST] = true;
	}
	return values_[NDX_GMST];
}

double ATimeSpace::GST() {
	if (valid_[NDX_GST]) {

	}
	return values_[NDX_GST];
}

double ATimeSpace::LMST() {
	if (!valid_[NDX_LMST]) {
		values_[NDX_LMST] = crcmod(GMST() + lon_);
		valid_[NDX_LMST] = true;
	}
	return values_[NDX_LMST];
}

double ATimeSpace::LST() {
	if (valid_[NDX_LST]) {
		values_[NDX_LST] = crcmod(GST() + lon_);
		valid_[NDX_LST] = true;
	}
	return values_[NDX_LST];
}

// 由UT1计算地球自转角
double ATimeSpace::ERA() {
	if (valid_[NDX_ERA]) {
		double ut1 = UT1();
		double fd, t;

		fd = ut1 - int(ut1);
		t  = ut1 - MJD2K;
		values_[NDX_ERA] = cycmod(0.5 + fd + 0.7790572732640 + 2.73781191135448E-3 * t, 1.0) * A2PI;
		valid_[NDX_ERA] = true;
	}
	return values_[NDX_ERA];
}
/////////////////////////////////////////////////////////////////////////////
/*---------------- 位置/坐标 ----------------*/
void ATimeSpace::calc_aberration_coef() {
	if (!valid_[NDX_AB]) {
		/* 计算周年光行差改正系数 */
		double t = JulianCentury();
		double L2 = 3.1761467 + 1021.3285546 * t;
		double L3 = 1.7534703 +  628.3075849 * t;
		double L4 = 6.2034809 +  334.0612431 * t;
		double L5 = 0.5995465 +   52.9690965 * t;
		double L6 = 0.8740168 +   21.3299095 * t;
		double L7 = 5.4812939 +    7.4781599 * t;
		double L8 = 5.3118863 +    3.8133036 * t;
		double LA = 3.8103444 + 8399.6847337 * t;
		double D  = 5.1984667 + 7771.3771486 * t;
		double MA = 2.3555559 + 8328.6914289 * t;
		double F  = 1.6279052 + 8433.4661601 * t;
		double v;

		ab_sin_[0]  = sin(v = L3);						ab_cos_[0]  = cos(v);
		ab_sin_[1]  = sin(v = 2 * L3);					ab_cos_[1]  = cos(v);
		ab_sin_[2]  = sin(v = L5);						ab_cos_[2]  = cos(v);
		ab_sin_[3]  = sin(v = LA);						ab_cos_[3]  = cos(v);
		ab_sin_[4]  = sin(v = 3 * L3);					ab_cos_[4]  = cos(v);
		ab_sin_[5]  = sin(v = L6);						ab_cos_[5]  = cos(v);
		ab_sin_[6]  = sin(v = F);						ab_cos_[6]  = cos(v);
		ab_sin_[7]  = sin(v = LA + MA);					ab_cos_[7]  = cos(v);
		ab_sin_[8]  = sin(v = 2 * L5);					ab_cos_[8]  = cos(v);
		ab_sin_[9]  = sin(v = 2 * L3 - L5);				ab_cos_[9]  = cos(v);
		ab_sin_[10] = sin(v = 3 * L3 - 8 * L4 + 3 * L5);ab_cos_[10] = cos(v);
		ab_sin_[11] = sin(v = 5 * L3 - 8 * L4 + 3 * L5);ab_cos_[11] = cos(v);
		ab_sin_[12] = sin(v = 2 * L2 - L3);				ab_cos_[12] = cos(v);
		ab_sin_[13] = sin(v = L2);						ab_cos_[13] = cos(v);
		ab_sin_[14] = sin(v = L7);						ab_cos_[14] = cos(v);
		ab_sin_[15] = sin(v = L3 - 2 * L5);				ab_cos_[15] = cos(v);
		ab_sin_[16] = sin(v = L8);						ab_cos_[16] = cos(v);
		ab_sin_[17] = sin(v = L3 + L5);					ab_cos_[17] = cos(v);
		ab_sin_[18] = sin(v = 2 * (L2 - L3));			ab_cos_[18] = cos(v);
		ab_sin_[19] = sin(v = L3 - L5);					ab_cos_[19] = cos(v);
		ab_sin_[20] = sin(v = 4 * L3);					ab_cos_[20] = cos(v);
		ab_sin_[21] = sin(v = 3 * L3 - 2 * L5);			ab_cos_[21] = cos(v);
		ab_sin_[22] = sin(v = L2 - 2 * L3);				ab_cos_[22] = cos(v);
		ab_sin_[23] = sin(v = 2 * L2 - 3 * L3);			ab_cos_[23] = cos(v);
		ab_sin_[24] = sin(v = 2 * L6);					ab_cos_[24] = cos(v);
		ab_sin_[25] = sin(v = 2 * (L2 - 2 * L3));		ab_cos_[25] = cos(v);
		ab_sin_[26] = sin(v = 3 * L3 - 2 * L4);			ab_cos_[26] = cos(v);
		ab_sin_[27] = sin(v = LA + 2 * D - MA);			ab_cos_[27] = cos(v);
		ab_sin_[28] = sin(v = 8 * L2 - 12 * L3);		ab_cos_[28] = cos(v);
		ab_sin_[29] = sin(v = 8 * L2 - 14 * L3);		ab_cos_[29] = cos(v);
		ab_sin_[30] = sin(v = 2 * L4);					ab_cos_[30] = cos(v);
		ab_sin_[31] = sin(v = 3 * L2 - 4 * L3);			ab_cos_[31] = cos(v);
		ab_sin_[32] = sin(v = 2 * (L3 - L5));			ab_cos_[32] = cos(v);
		ab_sin_[33] = sin(v = 3 * (L2 - L3));			ab_cos_[33] = cos(v);
		ab_sin_[34] = sin(v = 2 * (L3 - L4));			ab_cos_[34] = cos(v);
		ab_sin_[35] = sin(v = LA - 2 * D);				ab_cos_[35] = cos(v);

		coef_aab_[0][0] = -1719914 -   2 * t;
		coef_aab_[0][2] =       25 -  13 * t;
		coef_aab_[0][3] =  1578089 + 156 * t;
		coef_aab_[0][4] =       10 +  32 * t;
		coef_aab_[0][5] =   684185 - 358 * t;
		coef_aab_[1][0] =     6434 + 141 * t;
		coef_aab_[1][1] =    28007 - 107 * t;
		coef_aab_[1][2] =    25697 -  95 * t;
		coef_aab_[1][3] =    -5904 - 130 * t;
		coef_aab_[1][4] =    11141 -  48 * t;
		coef_aab_[1][5] =    -2559 -  55 * t;
		coef_aab_[4][0] =      486 -   5 * t;
		coef_aab_[4][1] =     -236 -   4 * t;
		coef_aab_[4][2] =     -216 -   4 * t;
		coef_aab_[4][3] =     -446 +   5 * t;

		valid_[NDX_AB] = true;
	}
}

void ATimeSpace::Sphere2Cart(double theta, double phi, double &x, double &y,
		double &z) {
	double cp = cos(phi);
	x = cos(theta) * cp;
	y = sin(theta) * cp;
	z = sin(phi);
}

void ATimeSpace::Cart2Sphere(double x, double y, double z, double &theta,
		double &phi) {
	double d2 = x * x + y * y;
	theta = d2 == 0.0 ? 0.0 : atan2(y, x);
	phi = z = 0.0 ? 0.0 : atan2(z, sqrt(d2));
	theta = crcmod(theta);
}

void ATimeSpace::AziAlt2Eq(double az, double el, double &ha, double dec) {
	double sa, ca, se, ce, sp, cp, x, y, z;

	sa = sin(az);
	ca = cos(az);
	se = sin(el);
	ce = cos(el);
	sp = sin(lat_);
	cp = cos(lat_);

	x = -ca * ce * sp + se * cp;
	y = -sa * ce;
	z = ca * ce * cp + se * sp;
	Cart2Sphere(x, y, z, ha, dec);
}

void ATimeSpace::Eq2AziAlt(double ha, double dec, double &az, double &el) {
	double sh, ch, sd, cd, sp, cp, x, y, z;

	sh = sin(ha);
	ch = cos(ha);
	sd = sin(dec);
	cd = cos(dec);
	sp = sin(lat_);
	cp = cos(lat_);

	x = -ch * cd * sp + sd * cp;
	y = -sh * cd;
	z = ch * cd * cp + sd * sp;
	Cart2Sphere(x, y, z, az, el);
}

double ATimeSpace::ParAngle(double ha, double dec) {
	double cp, cqsz, sqsz;

	cp = cos(lat_);
	sqsz = cp * sin(ha);
	cqsz = sin(lat_) * cos(dec) - cp * sin(dec) * cos(ha);
	return ((sqsz != 0.0 || cqsz != 0.0) ? atan2(sqsz, cqsz) : 0.0);
}

void ATimeSpace::AnnualAberration(double ra, double dec, double& d_ra, double& d_dec) {
	double c(17314463350.0);	// 量纲: 10^-8 AU/day
	double X(0.0), Y(0.0), Z(0.0);
	double cra  = cos(ra),  sra  = sin(ra);
	double cdec = cos(dec), sdec = sin(dec);

	calc_aberration_coef();
	/*
	 * X, Y, Z量纲: 10^-8 AU/day
	 */
	for (int i = 0; i < 36; ++i) {
		X += (coef_aab_[i][0] * ab_sin_[i] + coef_aab_[i][1] * ab_cos_[i]);
		Y += (coef_aab_[i][2] * ab_sin_[i] + coef_aab_[i][3] * ab_cos_[i]);
		Z += (coef_aab_[i][4] * ab_sin_[i] + coef_aab_[i][5] * ab_cos_[i]);
	}
	d_ra  = cdec > 1E-4 ? (Y * cra - X * sra) / c / cdec : 0.0;
	d_dec = (Z * cdec - (X * cra + Y * sra) * sdec) / c;
}
/*--------------------------------------------------------------------------*/
} /* namespace AstroUtil */
