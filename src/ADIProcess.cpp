/*
 * @file ADIProcess.cpp 天文数字图像处理定义文件
 * @date 2017-12-28
 * @version 0.1
 */

#include "ADIProcess.h"
#include "fits_handler.h"

using namespace astro_utility;

ADIProcess::ADIProcess() {
}

ADIProcess::~ADIProcess() {
}

void ADIProcess::SetZoneLight(const ZONE &zone) {
	zoneLight_ = zone;
}

void ADIProcess::SetParameter(ADIParameter *param) {
	param_ = *param;
}

bool ADIProcess::InitCombine(const char *pathroot) {
	if (!pathroot) return false;
	int n = strlen(pathroot);
	char ch;
	pathrootCombine_ = pathroot;
	filenameCombine_.clear();
	if ((ch = pathroot[n - 1]) != '/' && ch != '\\') pathrootCombine_.append(1, '/');

	return true;
}

void ADIProcess::AddFileCombine(const char *filename) {
	filenameCombine_.push_back(string(filename));
}

bool ADIProcess::load_image(const char *filepath, Image &image) {
	fits_handler hfit;
	int status(0);
	if (!hfit(filepath)) return false;
	image.SetDimension(hfit.width, hfit.height);
	image.exptime = hfit.exptime;

	fits_read_img(hfit(), TDOUBLE, 1, hfit.width * hfit.height, NULL, image.data, NULL, &status);
	image.isvalid = !status;
	return image.isvalid;
}

bool ADIProcess::LoadZero(const char *filepath) {
	return load_image(filepath, imgZero_);
}

bool ADIProcess::LoadDark(const char *filepath) {
	return load_image(filepath, imgDark_);
}

bool ADIProcess::LoadFlat(const char *filepath) {
	return load_image(filepath, imgFlat_);
}

bool ADIProcess::LoadImage(const char *filepath) {
	return load_image(filepath, imgObject_);
}

bool ADIProcess::ZeroCombine() {
	if (!filenameCombine_.size()) return false;

	return true;
}

bool ADIProcess::DarkCombine() {
	if (!filenameCombine_.size()) return false;

	return true;
}

bool ADIProcess::FlatCombine() {
	if (!filenameCombine_.size()) return false;

	return true;
}

int ADIProcess::ProcessImage() {
	if (!imgObject_.isvalid) return 1;
	if (!pre_process()) return 2;

	return 0;
}

bool ADIProcess::pre_process() {
	int pixels(imgObject_.wimg * imgObject_.himg), w(imgObject_.wimg), i, x, y, xs, xe, ys, ye;
	double t(imgObject_.exptime);
	double *ptr = imgObject_.data, *ptr1;

	if ((imgZero_.isvalid && !imgZero_.SameDimension(&imgObject_)) // 与本底不一致
			|| (imgDark_.isvalid && !imgDark_.SameDimension(&imgObject_)) // 与暗场不一致
			|| (imgFlat_.isvalid && !imgFlat_.SameDimension(&imgObject_)) // 与平场不一致
			|| (imgFlat_.isvalid && !imgZero_.isvalid)) { // 无本底, 有平场
		return false;
	}

	if (imgZero_.isvalid) {
		for (i = 0, ptr1 = imgZero_.data; i < pixels; ++i, ++ptr1) ptr[i] -= *ptr1;
	}
	if (imgDark_.isvalid) {
		for (i = 0, ptr1 = imgDark_.data; i < pixels; ++i, ++ptr1) ptr[i] -= (*ptr1 * t);
	}
	if (imgFlat_.isvalid) {
		if (!zoneLight_.width()) {// 未定义感光区
			for (i = 0, ptr1 = imgFlat_.data; i < pixels; ++i, ++ptr1) {
				if (*ptr1 > 1E-3) ptr[i] /= (*ptr1);
			}
		}
		else {
			xs = zoneLight_.xs;
			ys = zoneLight_.ys;
			xe = zoneLight_.xe;
			ye = zoneLight_.ye;

			ptr = imgObject_.data + ys * w;
			ptr1 = imgFlat_.data + ys * w;

			for (y = ys; y < ye; ++y, ptr += w, ptr1 += w) {// 遍历: 行
				for (x = xs; x < xe; ++x) {// 遍历: 列
					if (ptr1[x] > 1E-3) ptr[x] /= ptr1[x];
				}
			}
		}
	}

	return true;
}
