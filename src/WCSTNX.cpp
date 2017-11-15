/*
 * @file WCSTNX.cpp 定义文件, 基于非标准WCS格式TNX, 计算图像坐标与WCS坐标之间的对应关系
 * @version 0.1
 * @date 2017年11月9日
 * - 从FITS文件头加载WCS TNX参数项
 * - 从文本文件加载WCS TNX参数项
 * - 计算(x,y)对应的WCS坐标(ra, dec)
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "fits_handler.h"
#include "WCSTNX.h"

WCSTNX::WCSTNX() {

}

WCSTNX::~WCSTNX() {

}

int WCSTNX::LoadImage(const char* filepath) {
	fits_handler hfits;
	if (!hfits(filepath)) return -1; // FITS文件打开失败
	return 0;
}

bool WCSTNX::LoadText(const char* filepath) {
	FILE *fp = fopen(filepath, "r");
	if (!fp) return false;
	const int size(100);
	char line[size];
	char seps[] = " \t\r\n";
	char *token, *token1;

	while(!feof(fp)) {
		if (fgets(line, size, fp) == NULL) continue;
		token = strtok(line, seps);

		if (token[0] == 'x') {
//			if (!strcmp(token, "xrefmean")) param_.ref_xymean.x = atof(strtok(NULL, seps));
			if      (!strcmp(token, "xpixref")) param_.ref_xy.x = atof(strtok(NULL, seps));
//			else if (!strcmp(token, "xishift")) param_.shift.x = atof(strtok(NULL, seps));
//			else if (!strcmp(token, "xmag")) param_.mag.x = atof(strtok(NULL, seps)) * AS2R;
//			else if (!strcmp(token, "xrotation")) param_.rotation.x = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'y') {
//			if (!strcmp(token, "yrefmean")) param_.ref_xymean.y = atof(strtok(NULL, seps));
			if      (!strcmp(token, "ypixref")) param_.ref_xy.y = atof(strtok(NULL, seps));
//			else if (!strcmp(token, "ymag")) param_.mag.y = atof(strtok(NULL, seps)) * AS2R;
//			else if (!strcmp(token, "yrotation")) param_.rotation.y = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'l') {
//			if (!strcmp(token, "lngmean")) param_.ref_wcsmean.x = atof(strtok(NULL, seps)) * D2R;
//			else if (!strcmp(token, "latmean")) param_.ref_wcsmean.y = atof(strtok(NULL, seps)) * D2R;
			if      (!strcmp(token, "lngref"))  param_.ref_wcs.x = atof(strtok(NULL, seps)) * D2R;
			else if (!strcmp(token, "latref"))  param_.ref_wcs.y = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'e') {
//			if (!strcmp(token, "etashift")) param_.shift.y = atof(strtok(NULL, seps));
		}
		else if (token[0] == 'p') {
//			if (!strcmp(token, "pixsystem"))  param_.pixsystem  = strtok(NULL, seps);
//			else if (!strcmp(token, "projection")) param_.projection = strtok(NULL, seps);
		}
		else if (token[0] == 's') {
			int n;
			int xxorder, yxorder, xyorder, yyorder;
			int xterm, yterm;

			if (!strcmp(token, "surface1")) {
				param_surface& srfc = param_.surface1;
				int i, j(0);

				n = atoi(strtok(NULL, seps));
				for (i = 0; i < n && !feof(fp); ++i) {
					fgets(line, size, fp);
					token = strtok(line, seps);
					token1= strtok(NULL, seps);

					if (i == 0) { srfc.xsurface = atoi(token); srfc.ysurface = atoi(token1); }
					else if (i == 1) { xxorder = atoi(token);  yxorder = atoi(token1); }
					else if (i == 2) {
						xyorder = atoi(token);
						yyorder = atoi(token1);
						srfc.set_orderx(xxorder, xyorder);
						srfc.set_ordery(yxorder, yyorder);
					}
					else if (i == 3) {
						xterm = atoi(token);
						yterm = atoi(token1);
						srfc.set_xtermx(xterm);
						srfc.set_xtermy(yterm);
					}
					else if (i == 4) { srfc.xxmin = atof(token); srfc.yxmin = atof(token1); }
					else if (i == 5) { srfc.xxmax = atof(token); srfc.yxmax = atof(token1); }
					else if (i == 6) { srfc.xymin = atof(token); srfc.yymin = atof(token1); }
					else if (i == 7) { srfc.xymax = atof(token); srfc.yymax = atof(token1); }
					else {
						srfc.xcoef[j] = atof(token);
						srfc.ycoef[j] = atof(token1);
						++j;
					}
				}
				param_.valid1 = (n && i == n
						&& srfc.xxmax > srfc.xxmin && srfc.xymax > srfc.xymin
						&& srfc.yxmax > srfc.yxmin && srfc.yymax > srfc.yymin);
			}
			else if (!strcmp(token, "surface2")) {
				param_surface& srfc = param_.surface2;
				int i(0), j(0);

				n = atoi(strtok(NULL, seps));
				for (i = 0; i < n && !feof(fp); ++i) {
					fgets(line, size, fp);
					token = strtok(line, seps);
					token1= strtok(NULL, seps);

					if (i == 0) { srfc.xsurface = atoi(token); srfc.ysurface = atoi(token1); }
					else if (i == 1) { xxorder = atoi(token);  yxorder = atoi(token1); }
					else if (i == 2) {
						xyorder = atoi(token);
						yyorder = atoi(token1);
						srfc.set_orderx(xxorder, xyorder);
						srfc.set_ordery(yxorder, yyorder);
					}
					else if (i == 3) {
						xterm = atoi(token);
						yterm = atoi(token1);
						srfc.set_xtermx(xterm);
						srfc.set_xtermy(yterm);
					}
					else if (i == 4) { srfc.xxmin = atof(token); srfc.yxmin = atof(token1); }
					else if (i == 5) { srfc.xxmax = atof(token); srfc.yxmax = atof(token1); }
					else if (i == 6) { srfc.xymin = atof(token); srfc.yymin = atof(token1); }
					else if (i == 7) { srfc.xymax = atof(token); srfc.yymax = atof(token1); }
					else {
						srfc.xcoef[j] = atof(token);
						srfc.ycoef[j] = atof(token1);
						++j;
					}
				}
				param_.valid2 = (n && i == n
						&& srfc.xxmax > srfc.xxmin && srfc.xymax > srfc.xymin
						&& srfc.yxmax > srfc.yxmin && srfc.yymax > srfc.yymin);
			}
		}
//		else if (!strcmp(token, "coosystem"))  param_.coosystem  = strtok(NULL, seps);
//		else if (!strcmp(token, "function"))   param_.function = strtok(NULL, seps);
	}
	fclose(fp);

	if (param_.valid1) {// 计算旋转矩阵
		param_surface &srfc = param_.surface1;
		param_.cd[0][0] = 2 * srfc.xcoef[1] * AS2D / (srfc.xxmax - srfc.xxmin);
		param_.cd[0][1] = 2 * srfc.xcoef[2] * AS2D / (srfc.xymax - srfc.xymin);
		param_.cd[1][0] = 2 * srfc.ycoef[1] * AS2D / (srfc.yxmax - srfc.yxmin);
		param_.cd[1][1] = 2 * srfc.ycoef[2] * AS2D / (srfc.yymax - srfc.yymin);
	}

	return (param_.valid1);
}

int WCSTNX::WriteImage(const char* filepath) {
	if (!param_.valid1) return -1; // 至少需要线性项
	fits_handler hfits;
	if (!hfits(filepath, 1)) return -2; // FITS文件打开失败

	string WCSASTRM = "ct4m.19990714T012701 (USNO-K V) by F. Valdes 1999-08-02";
	string CTYPE1 = "RA---TNX";
	string CTYPE2 = "DEC--TNX";
	string WAT0_001 = "system=image";
	string WAT1 = "wtype=tnx axtype=ra lngcor = ";
	string WAT2 = "wtype=tnx axtype=dec latcor = ";
	int WCSDIM = 2;
	double CRVAL1 = param_.ref_wcs.x * R2D;
	double CRVAL2 = param_.ref_wcs.y * R2D;
	double CRPIX1 = param_.ref_xy.x;
	double CRPIX2 = param_.ref_xy.y;
	double CD1_1  = param_.cd[0][0];
	double CD1_2  = param_.cd[0][1];
	double CD2_1  = param_.cd[1][0];
	double CD2_2  = param_.cd[1][1];
	int status(0);

	if (param_.valid2) {
		param_surface& srfc = param_.surface2;
		int nx = srfc.xncoef;
		int ny = srfc.yncoef;
		int n, i;
		char lngcor[2048], latcor[2048];
		char txtdbl[20];
		// 构建赤经修正项
		n = sprintf(lngcor, "\"%d %d %d %d ", srfc.xsurface, srfc.xxorder, srfc.xyorder, srfc.xxterm);

		output_precision_double(txtdbl, srfc.xxmin);
		n += sprintf(lngcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.xxmax);
		n += sprintf(lngcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.xymin);
		n += sprintf(lngcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.xymax);
		n += sprintf(lngcor + n, "%s ", txtdbl);

		for (i = 0; i < nx; ++i) {
			output_precision_double(txtdbl, srfc.xcoef[i]);
			n += sprintf(lngcor + n, "%s ", txtdbl);
		}
		n += sprintf(lngcor + n, "\"");
		lngcor[n] = 0;

		// 构建赤纬修正项
		n = sprintf(latcor, "\"%d %d %d %d ", srfc.ysurface, srfc.yxorder, srfc.yyorder, srfc.yxterm);

		output_precision_double(txtdbl, srfc.yxmin);
		n += sprintf(latcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.yxmax);
		n += sprintf(latcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.yymin);
		n += sprintf(latcor + n, "%s ", txtdbl);
		output_precision_double(txtdbl, srfc.yymax);
		n += sprintf(latcor + n, "%s ", txtdbl);

		for (i = 0; i < ny; ++i) {
			output_precision_double(txtdbl, srfc.ycoef[i]);
			n += sprintf(latcor + n, "%s ", txtdbl);
		}
		n += sprintf(latcor + n, "\"");
		latcor[n] = 0;

		// 合并修正项
		WAT1 += lngcor;
		WAT2 += latcor;
	}

	// 写入FITS头
	fits_update_key(hfits(), TSTRING, "WCSASTRM", (void*) WCSASTRM.c_str(), "WCS Source",                 &status);
	fits_update_key(hfits(), TINT,    "WCSDIM",   &WCSDIM,                  "WCS dimensionality",         &status);
	fits_update_key(hfits(), TSTRING, "CTYPE1",   (void*) CTYPE1.c_str(),   "Coordinate type",            &status);
	fits_update_key(hfits(), TSTRING, "CTYPE2",   (void*) CTYPE2.c_str(),   "Coordinate type",            &status);
	fits_update_key(hfits(), TDOUBLE, "CRVAL1",   &CRVAL1,                  "Coordinate reference value", &status);
	fits_update_key(hfits(), TDOUBLE, "CRVAL2",   &CRVAL2,                  "Coordinate reference value", &status);
	fits_update_key(hfits(), TDOUBLE, "CRPIX1",   &CRPIX1,                  "Coordinate reference pixel", &status);
	fits_update_key(hfits(), TDOUBLE, "CRPIX2",   &CRPIX2,                  "Coordinate reference pixel", &status);
	fits_update_key(hfits(), TDOUBLE, "CD1_1",    &CD1_1,                   "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD1_2",    &CD1_2,                   "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD2_1",    &CD2_1,                   "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD2_2",    &CD2_2,                   "Coordinate matrix",          &status);
	if (param_.valid2) {// 畸变改正项
		char item[70]; // 每行实际可存储数据68字节
		char keyword[10]; // 关键字
		int i, j, byteleft, byteitem(68);
		int len[] = { WAT1.size(), WAT2.size() };
		const char *head[] = { WAT1.c_str(), WAT2.c_str() };
		const char *ptr;

		fits_update_key(hfits(), TSTRING, "WAT0_001", (void*) WAT0_001.c_str(),  "Coordinate system", &status);
		for (j = 0; j < 2; ++j) {// 0: R.A.; 1: DEC.
			for (byteleft = len[j], i = 0, ptr = head[j]; byteleft; ptr += byteitem) {
				sprintf(keyword, "WAT%d_%03d", j + 1, ++i);
				if (byteleft > byteitem) {
					strncpy(item, ptr, byteitem);
					item[byteitem] = 0;
					fits_update_key(hfits(), TSTRING, keyword, (void*) item, "", &status);
				}
				else {
					fits_update_key(hfits(), TSTRING, keyword, (void*) ptr, "", &status);
				}
				byteleft = byteleft > byteitem ? byteleft - byteitem : 0;
			}
		}
	}

	return status;
}

int WCSTNX::XY2WCS(double x, double y, double& ra, double& dec) {
	if (!param_.valid1) return -1;
	double xi, eta, xi1, eta1;

	image_to_plane(x, y, xi, eta);
	if (param_.valid2) {
		correct(param_.surface2, x, y, xi1, eta1);
		xi += xi1;
		eta += eta1;
	}
	plane_to_wcs(xi, eta, ra, dec);

	return 0;
}

WCSTNX::param_tnx *WCSTNX::GetParam() const {
	return (param_tnx * const) &param_;
}

void WCSTNX::linear_array(double x, int order, double* ptr) {
	int i;

	ptr[0] = 1.0;
	for (i = 1; i < order; ++i) {
		ptr[i] = x * ptr[i - 1];
	}
}

void WCSTNX::legendre_array(double x, double xmin, double xmax, int order, double* ptr) {
	int i;
	double xnorm = (2 * x - (xmax + xmin)) / (xmax - xmin);

	ptr[0] = 1.0;
	if (order > 1) ptr[1] = xnorm;
	for (i = 2; i < order; ++i) {
		ptr[i] = ((2 * i - 1) * xnorm * ptr[i - 1] - (i - 1) * ptr[i - 2]) / i;
	}
}

void WCSTNX::chebyshev_array(double x, double xmin, double xmax, int order, double* ptr) {
	int i;
	double xnorm = (2 * x - (xmax + xmin)) / (xmax - xmin);

	ptr[0] = 1.0;
	if (order > 1) ptr[1] = xnorm;
	for (i = 2; i < order; ++i) {
		ptr[i] = 2 * xnorm * ptr[i - 1] - ptr[i - 2];
	}
}

void WCSTNX::polyval_item(int type, int xorder, int yorder, double* x, double* y, int n, double* array) {
	double *ptr, t;
	int maxorder = xorder > yorder ? xorder : yorder;
	int i, j, imin(0), imax(xorder);

	for (j = 0, ptr = array; j < yorder; ++j) {
		if (j) {
			if (type == TNX_XNONE && imax != 1) imax = 1;
			else if (type == TNX_XHALF && (j + xorder) > maxorder) --imax;
		}

		for (i = imin, t = y[j]; i < imax; ++i, ++ptr) *ptr = x[i] * t;
	}
}

double WCSTNX::polysum(int n, double* coef, double* item) {
	double sum(0.0);

	for (int i = 0; i < n; ++i) sum += (coef[i] * item[i]);
	return sum;
}

void WCSTNX::correct(param_surface& surface, double x, double y, double& dx, double& dy) {
	// xi残差
	int xsurface(surface.xsurface), xxterm(surface.xxterm);
	int xxorder(surface.xxorder), xyorder(surface.xyorder);
	double xxmin(surface.xxmin), xxmax(surface.xxmax);
	double xymin(surface.xymin), xymax(surface.xymax);
	double *xx = surface.xx;
	double *xy = surface.xy;
	double *xxy = surface.xxy;

	if (xsurface == TNX_CHEB) {
		chebyshev_array(x, xxmin, xxmax, xxorder, xx);
		chebyshev_array(y, xymin, xymax, xyorder, xy);
	}
	else if (xsurface == TNX_LEG) {
		legendre_array(x, xxmin, xxmax, xxorder, xx);
		legendre_array(y, xymin, xymax, xyorder, xy);
	}
	else if (xsurface == TNX_POLY) {
		linear_array(x, xxorder, xx);
		linear_array(y, xyorder, xy);
	}
	polyval_item(xxterm, xxorder, xyorder, xx, xy, surface.xncoef, xxy);
	dx = polysum(surface.xncoef, surface.xcoef, xxy);
	// eta残差
	int ysurface(surface.ysurface), yxterm(surface.yxterm);
	int yxorder(surface.yxorder), yyorder(surface.yyorder);
	double yxmin(surface.yxmin), yxmax(surface.yxmax);
	double yymin(surface.yymin), yymax(surface.yymax);
	double *yx = surface.yx;
	double *yy = surface.yy;
	double *yxy = surface.yxy;

	if (ysurface == TNX_CHEB) {
		chebyshev_array(x, yxmin, yxmax, yxorder, yx);
		chebyshev_array(y, yymin, yymax, yyorder, yy);
	}
	else if (ysurface == TNX_LEG) {
		legendre_array(x, yxmin, yxmax, yxorder, yx);
		legendre_array(y, yymin, yymax, yyorder, yy);
	}
	else if (ysurface == TNX_POLY) {
		linear_array(x, yxorder, yx);
		linear_array(y, yyorder, yy);
	}
	polyval_item(yxterm, yxorder, yyorder, yx, yy, surface.yncoef, yxy);
	dy = polysum(surface.yncoef, surface.ycoef, yxy);

	dx *= AS2R;
	dy *= AS2R;
}

/*
 * @note 图像坐标转换为投影(中间)坐标
 */
void WCSTNX::image_to_plane(double x, double y, double& xi, double& eta) {
	if (param_.valid1) correct(param_.surface1, x, y, xi, eta);
	else {
		double dx(x - param_.ref_xy.x), dy(y - param_.ref_xy.y);
		xi  = (param_.cd[0][0] * dx + param_.cd[0][1] * dy) * D2R;
		eta = (param_.cd[1][0] * dx + param_.cd[1][1] * dy) * D2R;
	}
}

/*
 * @note 投影坐标转换为WCS(赤道)坐标
 */
void WCSTNX::plane_to_wcs(double xi, double eta, double &ra, double &dec) {
	double A0(param_.ref_wcs.x), D0(param_.ref_wcs.y);
	double fract = cos(D0) - eta * sin(D0);
	ra  = reduce(A0 + atan2(xi, fract), A2PI);
	dec = atan(((eta * cos(D0) + sin(D0)) * cos(ra - A0)) / fract);
}

int WCSTNX::output_precision_double(char *output, double value) {
	int n(0), pos(0), valid(0), intv;
	if (value < 0) {
		output[0] = '-';
		++pos;
		value = -value;
	}

	// 整数部分
	intv = int(value + AEPS);
	n = sprintf(output + pos, "%d.", intv);
	pos += n;
	valid = n - 1;
	// 小数部分
	while(valid < 17 && (value = (value - intv) * 10) > AEPS) {
		intv = int(value);
		sprintf(output + pos, "%d", intv);
		++pos;
		++valid;
	}
	output[pos] = 0;

	return n;
}
