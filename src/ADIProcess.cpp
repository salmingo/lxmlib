/*
 * @file ADIProcess.cpp 天文数字图像处理定义文件
 * @date 2017-12-28
 * @version 0.1
 */

#include "ADIProcess.h"
#include "fits_handler.h"

using namespace AstroUtil;

ADIProcess::ADIProcess() {
}

ADIProcess::~ADIProcess() {
}

void ADIProcess::SetZoneLightNumber(int n) {
	zoneLight_.n = n;
	zoneLight_.zones.clear();
}

void ADIProcess::add_zone(vector<ZONE> &zones, const ZONE *zone) {
	vector<ZONE>::iterator it;
	for (it = zones.begin(); it != zones.end() && !(*it == *zone); ++it);
	if (it == zones.end()) {
		ZONE znew = *zone;
		zones.push_back(znew);
	}
}

void ADIProcess::AddZoneLight(const ZONE &zone) {
	vector<ZONE> &zones = zoneLight_.zones;
	if (zones.size() < zoneLight_.n) add_zone(zones, &zone);
}

void ADIProcess::SetZoneOverscanNumber(int n) {
	zoneOver_.n = n;
	zoneOver_.zones.clear();
}

void ADIProcess::AddZoneOverscan(const ZONE &zone) {
	vector<ZONE> &zones = zoneOver_.zones;
	if (zones.size() < zoneOver_.n) add_zone(zones, &zone);
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
	int pixels(imgObject_.wimg * imgObject_.himg), i;
	double t(imgObject_.exptime);
	double *ptr = imgObject_.data, *ptr1;

	if (imgZero_.isvalid) {
		if (!imgZero_.SameDimension(&imgObject_)) return false;
		for (i = 0, ptr1 = imgZero_.data; i < pixels; ++i, ++ptr1) ptr[i] -= *ptr1;
	}
	if (imgDark_.isvalid) {
		if (!imgDark_.SameDimension(&imgObject_)) return false;
		for (i = 0, ptr1 = imgDark_.data; i < pixels; ++i, ++ptr1) ptr[i] -= (*ptr1 * t);
	}
	if (imgFlat_.isvalid) {
		if (!imgFlat_.SameDimension(&imgObject_)		// 图像尺寸不一致
				|| !imgZero_.isvalid)  // 未加载本底
			return false;
		for (i = 0, ptr1 = imgFlat_.data; i < pixels; ++i, ++ptr1) {
			if (*ptr1 > 1E-3) ptr[i] /= (*ptr1);
		}
	}

	return true;
}
