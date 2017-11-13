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
#include <longnam.h>
#include <fitsio.h>
#include "ADefine.h"
#include "AMath.h"
#include "WCSTNX.h"

using namespace AstroUtil;

WCSTNX::WCSTNX() {

}

WCSTNX::~WCSTNX() {

}

bool WCSTNX::LoadImage(const char* filepath) {
	return true;
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
			if (!strcmp(token, "xrefmean")) param_.ref_xymean.x = atof(strtok(NULL, seps));
			else if (!strcmp(token, "xpixref")) param_.ref_xy.x = atof(strtok(NULL, seps));
			else if (!strcmp(token, "xishift")) param_.shift.x = atof(strtok(NULL, seps));
			else if (!strcmp(token, "xmag")) param_.mag.x = atof(strtok(NULL, seps)) * AS2R;
			else if (!strcmp(token, "xrotation")) param_.rotation.x = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'y') {
			if (!strcmp(token, "yrefmean")) param_.ref_xymean.y = atof(strtok(NULL, seps));
			else if (!strcmp(token, "ypixref")) param_.ref_xy.y = atof(strtok(NULL, seps));
			else if (!strcmp(token, "ymag")) param_.mag.y = atof(strtok(NULL, seps)) * AS2R;
			else if (!strcmp(token, "yrotation")) param_.rotation.y = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'l') {
			if (!strcmp(token, "lngmean")) param_.ref_wcsmean.x = atof(strtok(NULL, seps)) * D2R;
			else if (!strcmp(token, "latmean")) param_.ref_wcsmean.y = atof(strtok(NULL, seps)) * D2R;
			else if (!strcmp(token, "lngref"))  param_.ref_wcs.x = atof(strtok(NULL, seps)) * D2R;
			else if (!strcmp(token, "latref"))  param_.ref_wcs.y = atof(strtok(NULL, seps)) * D2R;
		}
		else if (token[0] == 'e') {
			if (!strcmp(token, "etashift")) param_.shift.y = atof(strtok(NULL, seps));
		}
		else if (token[0] == 'p') {
			if (!strcmp(token, "pixsystem"))  param_.pixsystem  = strtok(NULL, seps);
			else if (!strcmp(token, "projection")) param_.projection = strtok(NULL, seps);
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
		else if (!strcmp(token, "coosystem"))  param_.coosystem  = strtok(NULL, seps);
		else if (!strcmp(token, "function"))   param_.function = strtok(NULL, seps);
	}
	fclose(fp);

	if (param_.valid1) {// 计算旋转矩阵
//		double xrot = param_.rotation.x * D2R - API;
//		double yrot = param_.rotation.y * D2R - API;
//		double xs = param_.mag.x * AS2D;
//		double ys = param_.mag.y * AS2D;
//		param_.cd[0][0] =  xs * cos(xrot);
//		param_.cd[0][1] = -ys * sin(yrot);
//		param_.cd[1][0] =  xs * sin(xrot);
//		param_.cd[1][1] =  ys * cos(yrot);
	}

	return (param_.valid1);
}

bool WCSTNX::WriteImage(const char* filepath) {
	return true;
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
	ProjectReverse(param_.ref_wcs.x, param_.ref_wcs.y, xi, eta, ra, dec);

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

		xi  = param_.cd[0][0] * dx + param_.cd[0][1] * dy;
		eta = param_.cd[1][0] * dx + param_.cd[1][1] * dy;
	}
}
