/*
 * @file WCSTNX.cpp 定义文件, 基于非标准WCS格式TNX, 计算图像坐标与WCS坐标之间的对应关系
 * @version 0.1
 * @date 2017年11月9日
 * - 从FITS文件头加载WCS TNX参数项
 * - 从文本文件加载WCS TNX参数项
 * - 计算(x,y)对应的WCS坐标(ra, dec)
 * - 计算(ra,dec)对应的图像坐标(x,y)
 */

#include <stdio.h>
#include <stdlib.h>
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

	char keyword[10], value[70], CTYPE1[10], CTYPE2[10];
	char strcor[2][2048], (*ptr)[2048];
	int status(0), i, j, n;

	param_.valid[0] = param_.valid[1] = false;

	// 检查TNX关键字
	fits_read_key(hfits(), TSTRING, "CTYPE1", CTYPE1, NULL, &status);
	fits_read_key(hfits(), TSTRING, "CTYPE2", CTYPE2, NULL, &status);
	if (status || strcasecmp(CTYPE1, "RA---TNX") || strcasecmp(CTYPE2, "DEC--TNX"))
		return -2;

	// 检查一阶修正模型
	fits_read_key(hfits(), TDOUBLE, "CRVAL1", &param_.ref_wcs.x, NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CRVAL2", &param_.ref_wcs.y, NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CRPIX1", &param_.ref_xy.x,  NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CRPIX2", &param_.ref_xy.y,  NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CD1_1",  &param_.cd[0],     NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CD1_2",  &param_.cd[1],     NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CD2_1",  &param_.cd[2],     NULL, &status);
	fits_read_key(hfits(), TDOUBLE, "CD2_2",  &param_.cd[3],     NULL, &status);
	if (status)
		return -3;
	param_.ref_wcs.x *= D2R;
	param_.ref_wcs.y *= D2R;
	if (invert_matrix())
		return -4;

	param_.valid[0] = true;
	fits_read_key(hfits(), TSTRING, "WAT1_001", value, NULL, &status);
	fits_read_key(hfits(), TSTRING, "WAT2_001", value, NULL, &status);
	if (status) // 无残差修正模型
		return 0;

	// 解析残差修正模型
	for (j = 1; j <= 2; ++j) {
		ptr = &strcor[j - 1];
		i = n = 0;
		while(!status) {
			sprintf(keyword, "WAT%d_%03d", j, ++i);
			fits_read_key(hfits(), TSTRING, keyword, value, NULL, &status);
			if (!status) {
				/* 作为字符串读出时, 结尾空格被删除, 需要手动填充, 避免数据被截断 */
				if (strlen(value) == 68) n += sprintf((*ptr) + n, "%s", value);
				else n += sprintf((*ptr) + n, "%s ", value);
			}
		}
		status = 0;
	}
	param_.valid[1] = !(resolve_tnxaxis(&strcor[0][0], &param_.tnx2[0])
			|| resolve_tnxaxis(&strcor[1][0], &param_.tnx2[1]));

	return param_.valid[1] ? 0 : -5;
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

		if      (!strcmp(token, "xpixref")) param_.ref_xy.x = atof(strtok(NULL, seps));
		else if (!strcmp(token, "ypixref")) param_.ref_xy.y = atof(strtok(NULL, seps));
		else if (!strcmp(token, "lngref"))  param_.ref_wcs.x = atof(strtok(NULL, seps)) * D2R;
		else if (!strcmp(token, "latref"))  param_.ref_wcs.y = atof(strtok(NULL, seps)) * D2R;
		else if (!strcmp(token, "wcsxirms"))  param_.errwcs.x = atof(strtok(NULL, seps));
		else if (!strcmp(token, "wcsetarms")) param_.errwcs.y = atof(strtok(NULL, seps));
		else if (!strcmp(token, "xirms"))     param_.errmid.x = atof(strtok(NULL, seps));
		else if (!strcmp(token, "etarms"))    param_.errmid.y = atof(strtok(NULL, seps));
		else if (token[0] == 's') {
			int srfc = !strcmp(token, "surface1") ? 1 : (!strcmp(token, "surface2") ? 2 : 0);
			if (srfc) {
				wcs_tnx *tnx = srfc == 1 ? &param_.tnx1[0] : &param_.tnx2[0];
				int i, j, n;

				n = atoi(strtok(NULL, seps));
				for (i = j = 0; i < n && !feof(fp); ++i) {
					fgets(line, size, fp);
					token = strtok(line, seps);
					token1= strtok(NULL, seps);

					if (i > 7) {
						tnx[0].coef[j] = atof(token);
						tnx[1].coef[j] = atof(token1);
						++j;
					}
					else if (i == 0) {
						tnx[0].function = int(atof(token)  + 0.5);
						tnx[1].function = int(atof(token1) + 0.5);
					}
					else if (i == 1) {
						tnx[0].set_orderx(int(atof(token)  + 0.5));
						tnx[1].set_orderx(int(atof(token1) + 0.5));
					}
					else if (i == 2) {
						tnx[0].set_ordery(int(atof(token)  + 0.5));
						tnx[1].set_ordery(int(atof(token1) + 0.5));
					}
					else if (i == 3) {
						tnx[0].set_xterm(int(atof(token)  + 0.5));
						tnx[1].set_xterm(int(atof(token1) + 0.5));
					}
					else if (i == 4) { tnx[0].xmin = atof(token); tnx[1].xmin = atof(token1); }
					else if (i == 5) { tnx[0].xmax = atof(token); tnx[1].xmax = atof(token1); }
					else if (i == 6) { tnx[0].ymin = atof(token); tnx[1].ymin = atof(token1); }
					else if (i == 7) { tnx[0].ymax = atof(token); tnx[1].ymax = atof(token1); }
				}
				param_.valid[srfc - 1] = n && i == n
						&& TNX_CHEBYSHEV <= tnx[0].function && tnx[0].function <= TNX_LINEAR
						&& TNX_CHEBYSHEV <= tnx[1].function && tnx[1].function <= TNX_LINEAR
						&& tnx[0].xorder > 0 && tnx[0].yorder > 0
						&& tnx[1].xorder > 0 && tnx[1].yorder > 0
						&& TNX_XNONE <= tnx[0].xterm && tnx[0].xterm <= TNX_XHALF
						&& TNX_XNONE <= tnx[1].xterm && tnx[1].xterm <= TNX_XHALF
						&& tnx[0].xmax > tnx[0].xmin && tnx[0].ymax > tnx[0].ymin
						&& tnx[1].xmax > tnx[1].xmin && tnx[1].ymax > tnx[1].ymin;
			}
		}
	}
	fclose(fp);

	if (param_.valid[0]) {// 计算旋转矩阵
		wcs_tnx *tnx = &param_.tnx1[0];
		param_.cd[0] = 2 * tnx[0].coef[1] * AS2D / (tnx[0].xmax - tnx[0].xmin);
		param_.cd[1] = 2 * tnx[0].coef[2] * AS2D / (tnx[0].ymax - tnx[0].ymin);
		param_.cd[2] = 2 * tnx[1].coef[1] * AS2D / (tnx[1].xmax - tnx[1].xmin);
		param_.cd[3] = 2 * tnx[1].coef[2] * AS2D / (tnx[1].ymax - tnx[1].ymin);
		if (invert_matrix()) param_.valid[0] = false;
	}

	return (param_.valid[0]);
}

int WCSTNX::WriteImage(const char* filepath) {
	if (!param_.valid[0]) return -1; // 至少需要线性项
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
	double CD1_1  = param_.cd[0];
	double CD1_2  = param_.cd[1];
	double CD2_1  = param_.cd[2];
	double CD2_2  = param_.cd[3];
	int status(0);

	if (param_.valid[1]) {
		wcs_tnx *tnx = &param_.tnx2[0];
		int n, nc, i, j;
		char strcor[2][2048];
		char txtdbl[20];
		char (*ptr)[2048];
		for (j = 0, tnx = &param_.tnx2[0]; j < 2; ++j, ++tnx) {
			ptr = &strcor[j];
			n = sprintf(*ptr, "\"%d %d %d %d ", tnx->function, tnx->xorder, tnx->yorder, tnx->xterm);
			output_precision_double(txtdbl, tnx->xmin);
			n += sprintf((*ptr) + n, "%s ", txtdbl);
			output_precision_double(txtdbl, tnx->xmax);
			n += sprintf((*ptr) + n, "%s ", txtdbl);
			output_precision_double(txtdbl, tnx->ymin);
			n += sprintf((*ptr) + n, "%s ", txtdbl);
			output_precision_double(txtdbl, tnx->ymax);
			n += sprintf((*ptr) + n, "%s ", txtdbl);

			nc = tnx->ncoef;
			for (i = 0; i < nc; ++i) {
				output_precision_double(txtdbl, tnx->coef[i]);
				n += sprintf((*ptr) + n, "%s ", txtdbl);
			}
			n += sprintf((*ptr) + n, "\"");
			(*ptr)[n] = 0;
		}

		// 合并修正项
		WAT1 += strcor[0];
		WAT2 += strcor[1];
	}

	// 写入FITS头
	fits_update_key(hfits(), TSTRING, "WCSASTRM", (void*) WCSASTRM.c_str(), "WCS Source",         &status);
	fits_update_key(hfits(), TINT,    "WCSDIM",   &WCSDIM,                  "WCS dimensionality", &status);
	fits_update_key(hfits(), TSTRING, "CTYPE1",   (void*) CTYPE1.c_str(),   "Coordinate type",    &status);
	fits_update_key(hfits(), TSTRING, "CTYPE2",   (void*) CTYPE2.c_str(),   "Coordinate type",    &status);
	fits_update_key(hfits(), TDOUBLE, "CRVAL1",   &CRVAL1, "Coordinate reference value", &status);
	fits_update_key(hfits(), TDOUBLE, "CRVAL2",   &CRVAL2, "Coordinate reference value", &status);
	fits_update_key(hfits(), TDOUBLE, "CRPIX1",   &CRPIX1, "Coordinate reference pixel", &status);
	fits_update_key(hfits(), TDOUBLE, "CRPIX2",   &CRPIX2, "Coordinate reference pixel", &status);
	fits_update_key(hfits(), TDOUBLE, "CD1_1",    &CD1_1,  "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD1_2",    &CD1_2,  "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD2_1",    &CD2_1,  "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "CD2_2",    &CD2_2,  "Coordinate matrix",          &status);
	fits_update_key(hfits(), TDOUBLE, "WCSERR_1", &param_.errwcs.x,  "WCS fit error",    &status);
	fits_update_key(hfits(), TDOUBLE, "WCSERR_2", &param_.errwcs.y,  "WCS fit error",    &status);
	fits_update_key(hfits(), TDOUBLE, "MIDERR_1", &param_.errmid.x,  "WCS fit error",    &status);
	fits_update_key(hfits(), TDOUBLE, "MIDERR_2", &param_.errmid.y,  "WCS fit error",    &status);
	if (param_.valid[1]) {// 畸变改正项
		char item[70]; // 每行实际可存储数据68字节
		char keyword[10]; // 关键字
		int i, j, byteleft, byteitem(68);
		int len[] = { WAT1.size(), WAT2.size() };
		const char *head[] = { WAT1.c_str(), WAT2.c_str() };
		const char *ptr;

		fits_update_key(hfits(), TSTRING, "WAT0_001", (void*) WAT0_001.c_str(), "Coordinate system", &status);
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
	if (!param_.valid[0]) return -1;
	double xi, eta;

	image_to_plane(x, y, xi, eta);
	if (param_.valid[1]) {
		xi  += param_.tnx2[0].project_reverse(x, y);
		eta += param_.tnx2[1].project_reverse(x, y);
	}
	plane_to_wcs(xi, eta, ra, dec);

	return 0;
}

int WCSTNX::WCS2XY(double ra, double dec, double &x, double &y) {
	if (!param_.valid[0]) return -1;
	double xi, eta;

	wcs_to_plane(ra, dec, xi, eta);
	plane_to_image(xi, eta, x, y);

	if (param_.valid[1]) {
		double x1, y1, dxi, deta;
		int cnt(0);

		do {
			x1 = x, y1 = y;
			dxi  = param_.tnx2[0].project_reverse(x, y);
			deta = param_.tnx2[1].project_reverse(x, y);
			plane_to_image(xi - dxi, eta - deta, x, y);
		} while(fabs(x1 - x) > 1E-3 && fabs(y1 - y) > 1E-3 && ++cnt < 15);
	}

	return 0;
}

WCSTNX::param_tnx *WCSTNX::GetParam() const {
	return (param_tnx * const) &param_;
}

/*
 * @note 图像坐标转换为投影(中间)坐标
 */
void WCSTNX::image_to_plane(double x, double y, double& xi, double& eta) {
	double dx(x - param_.ref_xy.x), dy(y - param_.ref_xy.y);
	xi  = (param_.cd[0] * dx + param_.cd[1] * dy) * D2R;
	eta = (param_.cd[2] * dx + param_.cd[3] * dy) * D2R;
}

/*
 * @note 投影坐标转换为WCS(赤道)坐标
 */
void WCSTNX::plane_to_wcs(double xi, double eta, double &ra, double &dec) {
	double ra0(param_.ref_wcs.x), dec0(param_.ref_wcs.y);
	double fract = cos(dec0) - eta * sin(dec0);
	ra  = reduce(ra0 + atan2(xi, fract), A2PI);
	dec = atan2(((eta * cos(dec0) + sin(dec0)) * cos(ra - ra0)), fract);
}

void WCSTNX::plane_to_image(double xi, double eta, double &x, double &y) {
	xi  *= R2D;
	eta *= R2D;
	x = (param_.ccd[0] * xi + param_.ccd[1] * eta) + param_.ref_xy.x;
	y = (param_.ccd[2] * xi + param_.ccd[3] * eta) + param_.ref_xy.y;
}

void WCSTNX::wcs_to_plane(double ra, double dec, double &xi, double &eta) {
	double ra0(param_.ref_wcs.x), dec0(param_.ref_wcs.y);
	double fract = sin(dec0) * sin(dec) + cos(dec0) * cos(dec) * cos(ra - ra0);
	xi  = cos(dec) * sin(ra - ra0) / fract;
	eta = (cos(dec0) * sin(dec) - sin(dec0) * cos(dec) * cos(ra - ra0)) / fract;
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

int WCSTNX::resolve_tnxaxis(char *strcor, wcs_tnx *tnx)  {
	char	 *pstr, *ptr;
	const char seps[] = " ";
	const char flags[] = "1234567890-+.";
	int xorder, yorder, xterm, i;

	if (NULL == (pstr = strpbrk(strcor, flags)))
		return 1;

	tnx->function = int(atof(strtok_r(pstr, seps, &ptr)) + 0.5);
	xorder        = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? int(atof(pstr) + 0.5) : 0;
	yorder        = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? int(atof(pstr) + 0.5) : 0;
	xterm         = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? int(atof(pstr) + 0.5) : 0;
	tnx->xmin     = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? atof(pstr) : 0.0;
	tnx->xmax     = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? atof(pstr) : 0.0;
	tnx->ymin     = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? atof(pstr) : 0.0;
	tnx->ymax     = (pstr = strtok_r(NULL, seps, &ptr)) != NULL ? atof(pstr) : 0.0;
	if (TNX_CHEBYSHEV > tnx->function || TNX_LINEAR < tnx->function
			|| xorder <= 0 || yorder <= 0
			|| TNX_XNONE > xterm || TNX_XHALF < xterm
			|| tnx->xmax <= tnx->xmin || tnx->ymax < tnx->ymin)
		return 2;

	tnx->set_orderx(xorder);
	tnx->set_ordery(yorder);
	tnx->set_xterm(xterm);

	for (i = 0; i < tnx->ncoef && (pstr = strtok_r(NULL, seps, &ptr)) != NULL; ++i)
		tnx->coef[i] = atof(pstr);

	return (i == tnx->ncoef) ? 0 : 3;
}

int WCSTNX::invert_matrix() {
	int lc;
	int *le, *lep;
	double s,t,tq = 0., zr = 1.e-15;
	double *pa, *pd, *ps, *p, *q;
	double *q0;
	int i, j, k, m;
	int N(2);
	double *A = param_.ccd;

	memcpy(A, param_.cd, sizeof(double) * 4);
	le = lep = (int *) malloc(N * sizeof(int));
	q0 = (double *) malloc(N * sizeof(double));

	for(j = 0, pa = pd = A; j < N ; ++j, ++pa, pd += N + 1) {
		if( j > 0) {
			for(i = 0, q = q0, p = pa; i < N; ++i, ++q, p += N) *q = *p;
			for(i = 1; i < N; ++i) {
				lc = i < j ? i : j;
				for(k = 0, p = pa + i * N - j, q = q0,t = 0.; k < lc ;++k, ++p, ++q) t += *p * *q;
				q0[i] -= t;
			}
			for(i = 0, q = q0, p = pa; i < N; ++i, p += N, ++q) *p = *q;
		}

		s = fabs(*pd);
		lc = j;

		for(k = j + 1, ps = pd; k < N; ++k) {
			if((t = fabs(*(ps += N))) > s) {
				s = t;
				lc = k;
			}
		}

		tq = tq > s ? tq : s;
		if(s < zr * tq) {
			free(q0);
			free(le);
			return -1;
		}

		*lep++ = lc;
		if(lc != j) {
			for(k = 0, p = A + N * j, q = A + N * lc; k < N; ++k, ++p, ++q) {
				t = *p;
				*p = *q;
				*q = t;
			}
		}

		for(k = j + 1, ps = pd, t=1./ *pd; k < N; ++k) *(ps += N) *= t;
		*pd = t;
	}

	for(j = 1, pd = ps = A; j < N; ++j) {
		for(k = 0, pd += N + 1, q = ++ps; k < j; ++k, q += N) *q *= *pd;
	}

	for(j = 1, pa = A; j < N; ++j) {
		++pa;
		for(i = 0, q = q0, p = pa; i < j; ++i, p += N, ++q) *q = *p;
		for(k = 0; k < j; ++k) {
			t=0.;
			for(i = k, p = pa + k * N + k - j, q = q0 + k; i<j; ++i, ++p, ++q) t -= *p * *q;
			q0[k] = t;
		}
		for(i = 0, q = q0, p = pa; i < j; ++i, p += N, ++q) *p = *q;
	}

	for(j = N - 2, pd = pa = A + N * N - 1; j >= 0; --j) {
		--pa;
		pd -= N + 1;

		for(i = 0, m = N - j - 1, q = q0, p = pd + N; i < m; ++i, p += N, ++q) *q = *p;
		for(k = N - 1, ps = pa; k > j; --k, ps -= N) {
			t = -(*ps);
			for(i = j + 1, p = ps, q = q0; i < k; ++i, ++q) t -= *++p * *q;
			q0[--m] = t;
		}
		for(i = 0, m = N - j - 1, q = q0, p = pd + N; i < m; ++i, p += N) *p = *q++;
	}

	for(k = 0, pa = A; k < N - 1; ++k, ++pa) {
		for(i = 0, q = q0, p = pa; i < N; ++i, p += N) *q++ = *p;
		for(j = 0, ps =A; j < N; ++j, ps += N) {
			if(j > k) { t = 0.;    p = ps + j;     i = j; }
			else      { t = q0[j]; p = ps + k + 1; i = k + 1; }
			for(; i < N;) t += *p++ * q0[i++];
			q0[j] = t;
		}

		for(i = 0, q = q0, p = pa; i < N; ++i, p += N) *p = *q++;
	}

	for(j = N - 2, --lep; j >= 0;--j) {
		for(k = 0, p = A + j, q = A + *(--lep); k < N; ++k, p += N, q += N) {
			t = *p;
			*p = *q;
			*q = t;
		}
	}

	free(le);
	free(q0);
	return 0;
}
