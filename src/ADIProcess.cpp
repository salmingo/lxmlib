/*
 * @file ADIProcess.cpp 天文数字图像处理定义文件
 * @date 2017-12-28
 * @version 0.1
 */

#include "ADIProcess.h"

using namespace AstroUtil;

ADIProcess::ADIProcess() {
}

ADIProcess::~ADIProcess() {
}

void ADIProcess::SetZoneLightNumber(int n) {
	zoneLight_.n = n;
	zoneLight_.zones.clear();
}

void ADIProcess::AddZoneLight(const ZONE &zone) {
	vector<ZONE> &zones = zoneLight_.zones;
	if (zones.size() < zoneLight_.n) {
		vector<ZONE>::iterator it;
		for (it = zones.begin(); it != zones.end() && zone != *it; ++it);
		if (it == zones.end()) {
			ZONE znew = zone;
			zones.push_back(znew);
		}
	}
}

void ADIProcess::SetZoneOverscanNumber(int n) {
	zoneOver_.n = n;
	zoneOver_.zones.clear();
}

void ADIProcess::AddZoneOverscan(const ZONE &zone) {
	vector<ZONE> &zones = zoneOver_.zones;
	if (zones.size() < zoneOver_.n) {
		vector<ZONE>::iterator it;
		for (it = zones.begin(); it != zones.end() && zone != *it; ++it);
		if (it == zones.end()) {
			ZONE znew = zone;
			zones.push_back(znew);
		}
	}
}

void ADIProcess::SetParameter(Parameter *param) {
	param_ = *param;
}
