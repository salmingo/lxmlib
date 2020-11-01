/*
 * @file ATimeSpace.h 天文时空转换函数接口
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
double frac(double x) {// 计算x的小数部分
	return x - floor(x);
}

double cycmod(double x, double T) {// 将x\T的余数调整到1x正周期内
	double r = fmod(x, T);
	return r < 0 ? (r + T) : r;
}

double crcmod(double x) {// 将x\2π的余数调整到[0, 2π)范围内
	return cycmod(x, A2PI);
}
/////////////////////////////////////////////////////////////////////////////

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

bool ATimeSpace::GetYDays(int &iy, double &ydays) {
	int im, id;
	double mjd, fd;

	if (!ModifiedJulianDay(mjd)) return false;
	MJD2Cal(mjd, iy, im, id, fd);
	ydays = ymd2ydas(iy, im, id) + fd;

	return true;
}

bool ATimeSpace::ModifiedJulianDay(double & mjd) {
	if (!valid_[NDX_MJD])
		return false;
	mjd = values_[NDX_MJD];
	return true;
}

bool ATimeSpace::JulianDay(double & jd) {
	if (!valid_[NDX_MJD])
		return false;
	if (!valid_[NDX_JD]) {
		values_[NDX_JD] = values_[NDX_MJD] + MJD0;
		valid_[NDX_JD] = true;
	}
	jd = values_[NDX_JD];

	return true;
}

bool ATimeSpace::JulianCentury(double &jc) {
	if (!valid_[NDX_MJD])
		return false;
	if (!valid_[NDX_JC]) {
		values_[NDX_JC] = (values_[NDX_MJD] - MJD2K) / DAYSJC;
		valid_[NDX_JC] = true;
	}
	jc = values_[NDX_JC];

	return true;
}

bool ATimeSpace::Epoch(double & ep) {
	if (!valid_[NDX_MJD])
		return false;
	if (!valid_[NDX_EPOCH]) {
		values_[NDX_EPOCH] = (values_[NDX_MJD] - MJD2K) / DAYSJY + 2000.0;
		valid_[NDX_EPOCH] = true;
	}
	ep = values_[NDX_EPOCH];

	return true;
}

double ATimeSpace::EpochBessel(double mjd) {
	// 36524.68648 = B1900-J2000的天数
	return 1900.0 + (mjd - MJD2K + 36524.68648) / DAYSBY;
}

/////////////////////////////////////////////////////////////////////////////
/*---------------- 时间系统 ----------------*/
bool ATimeSpace::DeltaAT(double mjd, double &dat) {
	/*
	 * 当闰秒更新时, 需更改参数:
	 * - IYV变更为最后更新时间对应的年
	 * - changes[]添加更新数据
	 */
	const int IYV = 2017;
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

	MJD2Cal(mjd, iy, im, id, fd);
	if (iy < changes[0].iyear) return false;

	int i, m;
	m = 12 * iy + im;
	for (i = NDAT-1; i >=0; --i) {
		if (m >= (12 * changes[i].iyear + changes[i].month)) break;
	}
	dat = changes[i].delat;
	if (i < NERA1) dat += (values_[NDX_MJD] + fd - drift[i][0]) * drift[i][1];

	return true;
}

double ATimeSpace::DeltaUT2(double epb) {
	double t = A2PI * epb;
	double dut = 0.022 * sin(t) - 0.012 * cos(t) - 0.006 * sin(2 * t) + 0.007 * cos(2 * t);
	return dut;
}

// 计算与UTC对应的TAI的修正儒略日
bool ATimeSpace::TAI(double &tai) {
	if (valid_[NDX_TAI]) tai = values_[NDX_TAI];
	else {
		int mjd0;
		double mjd, fd;
		double dat0, dat12, dat24;
		double dlod, dleap;

		if (!ModifiedJulianDay(mjd)) return false;
		mjd0 = int(mjd);
		fd = mjd - mjd0;
		if (!DeltaAT(mjd0, dat0)
			|| !DeltaAT(mjd0 + 0.5, dat12)
			|| !DeltaAT(mjd0 + 1.0, dat24))
			return false;
		dlod = 2.0 * (dat12 - dat0);
		dleap = dat24 - (dat0 + dlod);
		fd *= (DAYSEC + dleap) / DAYSEC;
		fd *= (DAYSEC + dlod) / DAYSEC;
		tai = mjd0 + fd + dat0 / DAYSEC;

		values_[NDX_TAI] = tai;
		valid_[NDX_TAI] = true;
	}
	return true;
}

// 计算与UTC对应的UT1的修正儒略日
bool ATimeSpace::UT1(double &ut1) {
	if (valid_[NDX_UT1]) ut1 = values_[NDX_UT1];
	else {
		double mjd, tai;
		double dat;
		if (!ModifiedJulianDay(mjd)
			|| !TAI(tai)
			|| !DeltaAT(int(mjd), dat))
			return false;
		ut1 = tai + (dut1_ - dat) / DAYSEC;

		values_[NDX_UT1] = ut1;
		valid_[NDX_UT1] = true;
	}
	return true;
}

bool ATimeSpace::TT(double &tt) {
	if (valid_[NDX_TT]) tt = values_[NDX_TT];
	else {
		double tai;
		if (!TAI(tai)) return false;
		tt = tai + TTMTAI / DAYSEC;
		values_[NDX_TT] = tt;
		valid_[NDX_TT] = true;
	}
	return true;
}

bool ATimeSpace::GMST(double &gmst) {
	if (valid_[NDX_GMST]) gmst = values_[NDX_GMST];
	else {
		double tt, era, t;
		if (!TT(tt) || !ERA(era))
			return false;
		t = (tt - MJD2K) / DAYSJC;
		gmst = era +
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
	return true;
}

bool ATimeSpace::GST(double &gst) {
	if (valid_[NDX_GST]) gst = values_[NDX_GST];
	else {

	}
	return true;
}

bool ATimeSpace::LMST(double &lmst) {
	if (valid_[NDX_LMST]) lmst = values_[NDX_LMST];
	else {
		double gmst;
		if (!GMST(gmst)) return false;
		lmst = crcmod(gmst + lon_);
		values_[NDX_LMST] = lmst;
		valid_[NDX_LMST] = true;
	}
	return true;
}

bool ATimeSpace::LST(double &lst) {
	if (valid_[NDX_LST]) lst = values_[NDX_LST];
	else {
		double gst;
		if (!GMST(gst)) return false;
		lst = crcmod(gst + lon_);
		values_[NDX_LST] = lst;
		valid_[NDX_LST] = true;
	}
	return true;
}

// 由UT1计算地球自转角
bool ATimeSpace::ERA(double &era) {
	if (valid_[NDX_ERA]) era = values_[NDX_ERA];
	else {
		double ut1, fd, t;
		if (!UT1(ut1)) return false;
		fd = ut1 - int(ut1);
		t  = ut1 - MJD2K;
		era = cycmod(0.5 + fd + 0.7790572732640 + 2.73781191135448E-3 * t, 1.0) * A2PI;
		values_[NDX_ERA] = era;
		valid_[NDX_ERA] = true;
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////
/*---------------- 位置/坐标 ----------------*/
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
	double sh, ch, sd, cd, sp, cp, x, y, z, r, a;

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
/*--------------------------------------------------------------------------*/
} /* namespace AstroUtil */
