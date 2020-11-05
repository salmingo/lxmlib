/*!
 * @file ATimeSpace.h
 * @brief 天文时空转换函数接口
 * @date 2019-06-21
 * @version 1.0
 * @author 卢晓猛
 * @note
 * 时间标志及释义:
 * UTC  -- 协调世界时. Coordinated Universal Time
 * TAI  -- 国际原子时. International Atomic Time
 * UT1  -- 世界时. Universal Time
 * TT   -- 地球时. Terrestrial Time
 * TDT  -- 地球动力学事. Terrestrial Dynamic Time
 * TDB  -- 质心动力学时. Barycentric Dynamic Time
 * TCB  -- 质心坐标时. Barycentric Coordinate Time
 * TCG  -- 地心坐标时. Geocentric Coordinate Time
 *
 * @note
 * dUT1=UT1-UTC查询地址
 * https://datacenter.iers.org/availableVersions.php?id=17
 * |dUT1|<1s
 *
 * @note
 * <p>
 * 平位置=>视位置:
 * @li 自行
 * @li 光行差
 * @li 岁差
 * @li 章动
 * </p>
 */

#ifndef ATIMESPACE_H_
#define ATIMESPACE_H_

namespace AstroUtil {
/////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
class ATimeSpace {
public:
	ATimeSpace();
	virtual ~ATimeSpace();

protected:
	enum { // 基于历书的计算结果
		NDX_MJD,	//< 修正儒略日 /UTC
		NDX_ERA,	//< 与UTC对应的地球旋转角
		NDX_TAI,	//< 以修正儒略日表示的与UTC对应的TAI
		NDX_UT1,	//< 以修正儒略日表示的与UTC对应的UT1
		NDX_TT,		//< 以修正儒略日表示的与UTC对应的TT
		NDX_GMST,	//< 格林威治平恒星时
		NDX_GST,	//< 格林威治真恒星时
		NDX_LMST,	//< 本地平恒星时
		NDX_LST,	//< 本地真恒星时
		NDX_AB,		//< 周年光行差改正系数
		NDX_MAX
	};

protected:
	double lon_, lat_, alt_;	///< 测站位置. [lon, lat]量纲为弧度, alt量纲为米
	double dut1_;				///< UT1-UTC, 量纲: 秒
	bool valid_[NDX_MAX];		///< 数据有效标志
	double values_[NDX_MAX];	///< 计算结果
	static double coef_aab_[36][6];	///< 周年光行差改正系数
	double ab_sin_[36], ab_cos_[36];	///< 周年光行差改正系数

public:
	/*---------------- 常规参数 ----------------*/
	/*!
	 * @brief 设置测站位置
	 * @param lon 地理经度, 量纲: 角度
	 * @param lat 地理纬度, 量纲: 角度
	 * @param alt 海拔, 量纲: 米
	 */
	void SetSite(double lon, double lat, double alt);
	/*!
	 * @brief 设置UTC时间
	 * @param iy 年
	 * @param im 月
	 * @param id 日
	 * @param fd 日的小数部分
	 */
	void SetUTC(int iy, int im, int id, double fd = 0.0);
	/*!
	 * @brief 设置修正儒略日
	 * @param mjd 修正儒略日
	 */
	void SetMJD(double mjd);
	/*!
	 * @brief 设置儒略日
	 * @param jd 儒略日
	 */
	void SetJD(double jd);
	/*!
	 * @brief 设置历元
	 * @param ep 历元
	 */
	void SetEpoch(double ep);
	/*!
	 * @brief 设置年中的日数, 从1月1日0时开始计算
	 * @param iy     年
	 * @param ydays  日数
	 */
	void SetYDays(int iy, double ydays);
	/*!
	 * @brief 设置dUT1=UT1-UTC
	 * @param dut 时间偏差, 量纲: 秒
	 */
	void SetDeltaUT1(double dut);

protected:
	/*!
	 * @brief 由字符串解析小时数或角度数
	 * @note
	 * 由Str2H()或Str2D()调用
	 */
	bool hdresolve(const char* str, double& val);

public:
	/*---------------- 格式转换 ----------------*/
	/*!
	 * @brief 小时数转换为时分秒
	 * @param[in]  hour 小时数
	 * @param[out] hh   小时
	 * @param[out] mm   分
	 * @param[out] ss   秒
	 */
	void H2HMS(double hour, int & hh, int & mm, double & ss);
	/*!
	 * @brief 角度转换为度分秒
	 * @param[in]  deg   角度数
	 * @param[out] dd    角度
	 * @param[out] mm    角分
	 * @param[out] ss    角秒
	 * @param[out] sign  符号位. deg>=0时sign==1, 否则sign==-1
	 */
	void D2DMS(double deg, int & dd, int & mm, double & ss, int & sign);
	/*!
	 * @brief 将字符串格式小时数转换为实数
	 * @param[in]  str   字符串
	 * @param[out] hour  小时数
	 * @return
	 * 格式转换成功标志
	 * @note
	 * - 字符串时分秒之间使用:或空格作为分隔符. 当时分秒之间没有分隔符时, 时和分位长度必须为2字节
	 * - 小时数有效范围是: [0.0, 24.0)
	 */
	bool Str2H(const char* str, double & hour);
	/*!
	 * @brief 将字符串格式角度转换为实数
	 * @param[in]  str   字符串
	 * @param[out] deg   角度数
	 * @return
	 * 格式转换成功标志
	 * @note
	 * - 字符串度分秒之间使用:或空格作为分隔符. 当度分秒之间没有分隔符时, 度和分位长度必须为2字节
	 * - 角度数有效范围是: [-90.0, +90.0]
	 */
	bool Str2D(const char* str, double & deg);

	/*---------------- 日历/历书 ----------------*/
protected:
	/*!
	 * @brief 检查是否闰年
	 */
	bool leap_year(int iy);
	/*!
	 * @brief 计算与格力高里历对应的修正儒略日
	 * @param iy 年
	 * @param im 月
	 * @param id 日
	 * @param fd 日的小数部分
	 * @return
	 * 修正儒略日
	 */
	double cal2mjd(int iy, int im, int id, double fd);
	/*!
	 * @brief 年日数转换为对应的年月日
	 * @param iy    年
	 * @param ydays 年日数
	 * @param im    月
	 * @param id    日
	 */
	void ydays2ymd(int iy, int ydays, int &im, int &id);
	/*!
	 * @brief 年月日转换为对应的年日数
	 * @param iy  年
	 * @param im  月
	 * @param id  日
	 * @return
	 */
	int ymd2ydas(int iy, int im, int id);

public:
	/*!
	 * @brief 查看与修正儒略日对应的日期
	 */
	void MJD2Cal(double mjd, int &iy, int &im, int &id, double &fd);
	/*!
	 * @brief 查看与历书对应的年日数
	 */
	void GetYDays(double mjd, int &iy, double &ydays);
	/*!
	 * @brief 查看修正儒略日
	 */
	double ModifiedJulianDay();
	/*!
	 * @brief 查看儒略日
	 */
	double JulianDay();
	/*!
	 * @brief 计算与修正儒略日对应的儒略日
	 * @param mjd  修正儒略日
	 * @return
	 * 儒略日
	 */
	double JulianDay(double mjd);
	/*!
	 * @brief 查看相对J2000.0的儒略世纪
	 */
	double JulianCentury();
	/*!
	 * @brief 计算修正儒略日对应的儒略世纪
	 * @param mjd  修正儒略日
	 * @return
	 * 儒略世纪
	 */
	double JulianCentury(double mjd);
	/*!
	 * @brief 查看历元
	 */
	double Epoch();
	/*!
	 * @brief 计算修正儒略日对应的历元
	 * @param mjd  修正儒略日
	 * @return
	 * 历元
	 */
	double Epoch(double mjd);
	/*!
	 * @brief 计算对应的贝塞尔历元
	 */
	double EpochBessel(double mjd);

	/*---------------- 时间系统 ----------------*/
public:
	/*!
	 * @brief 计算闰秒
	 * @note
	 * DAT = TAI-UTC
	 */
	double DeltaAT(double mjd);
	/*!
	 * @brief 计算UT2-UT1
	 * @param epb 贝塞尔历元
	 * @return
	 * UT2-UT1, 量纲: 秒
	 */
	double DeltaUT2(double epb);
	/*!
	 * @brief 计算与UTC时间对应的TAI(国际原子时)
	 * @param tai TAI, 以修正儒略日表示
	 */
	double TAI();
	/*!
	 * @brief 计算与UTC时间对应的UT1
	 * @param ut1 世界时, 修正儒略日
	 */
	double UT1();
	/*!
	 * @brief 计算与UTC时间对应的TT
	 * @param tt 地球时/地面时, 修正儒略日
	 */
	double TT();
	/*!
	 * @brief 计算与输入UTC对应的格林威治平恒星时
	 * @param gmst 平恒星时, 量纲: 弧度
	 */
	double GMST();
	/*!
	 * @brief 计算与输入UTC对应的格林威治视恒星时
	 * @param gst 视恒星时, 量纲: 弧度
	 */
	double GST();
	/*!
	 * @brief 计算与输入UTC对应的本地平恒星时
	 * @param gmst 平恒星时, 量纲: 弧度
	 */
	double LMST();
	/*!
	 * @brief 计算与输入UTC对应的本地视恒星时
	 * @param gst 视恒星时, 量纲: 弧度
	 */
	double LST();

public:
	/*!
	 * @brief 计算地球自转角
	 * @param era 地球自转角, 量纲: 弧度
	 */
	double ERA();

	/*---------------- 位置/坐标 ----------------*/
protected:
	/*!
	 * @brief 计算周年光行差改正系数
	 */
	void calc_aberration_coef();

public:
	/*!
	 * @brief 球坐标=>直角坐标
	 * @param theta 在参考面中的相对X轴的夹角, 量纲: 弧度
	 * @param phi   相对参考面的倾角, 量纲: 弧度
	 * @param x     X
	 * @param y     Y
	 * @param z     Z
	 */
	void Sphere2Cart(double theta, double phi, double &x, double &y, double &z);
	/*!
	 * @brief 直角坐标=>球坐标
	 * @param x     X
	 * @param y     Y
	 * @param z     Z
	 * @param theta 在参考面中的
	 * @param phi   相对参考面的倾角
	 */
	void Cart2Sphere(double x, double y, double z, double &theta, double &phi);
	/*!
	 * @brief 地平坐标转换为赤道坐标
	 * @param az   方位角, 量纲: 弧度
	 * @param el   高度角, 量纲: 弧度
	 * @param ha   时角, 量纲: 弧度
	 * @param dec  赤纬, 量纲: 弧度
	 * @note
	 * - 方位角零点: 正北
	 */
	void AziAlt2Eq(double az, double el, double &ha, double dec);
	/*!
	 * @brief 赤道坐标转换为地平坐标
	 * @param ha   时角, 量纲: 弧度
	 * @param dec  赤纬, 量纲: 弧度
	 * @param az   方位角, 量纲: 弧度
	 * @param el   高度角, 量纲: 弧度
	 * @note
	 * - 方位角零点: 正北
	 */
	void Eq2AziAlt(double ha, double dec, double &az, double &el);
	/*!
	 * @brief 视差角
	 * @param ha  时角, 量纲: 弧度
	 * @param dec 赤纬, 量纲: 弧度
	 * @return
	 * 视差角, 量纲: 弧度
	 */
	double ParAngle(double ha, double dec);
	/*!
	 * @brief 计算周年光行差
	 * @param ra     赤经, 量纲: 弧度
	 * @param dec    赤纬, 量纲: 弧度
	 * @param d_ra   赤经偏差, 量纲: 弧度
	 * @param d_dec  赤经偏差, 量纲: 弧度
	 */
	void AnnualAberration(double ra, double dec, double& d_ra, double& d_dec);

public:
};
/*--------------------------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////
} /* namespace AstroUtil */

#endif /* ATIMESPACE_H_ */
