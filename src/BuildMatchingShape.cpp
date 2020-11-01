/**
 * @class BuildMatchingShape 创建用于坐标系匹配的模型
 * @version 0.1
 * @date 2020-11-01
 * @author 卢晓猛
 */

#include <string>
#include <algorithm>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "BuildMatchingShape.h"

using namespace std;
using namespace boost::property_tree;

/////////////////////////////////////////////////////////////////////////////
int ParamBMS::count_sample_max() {
	return count_sample_max_ < 10 ? 10 : count_sample_max_;
}

int ParamBMS::count_per_shape_min() {
	return count_per_shape_min_ < 1 ? 1 : count_per_shape_min_;
}

double ParamBMS::apex_angle() {
	double x = apex_angle_;
	if (x < 20.0 || x > 340.0) x = 0.0;
	return x;
}

double ParamBMS::distance_min() {
	double x = distance_min_;
	if (x < 0.01 || x > 0.2) x = 0.1;
	return x;
}

int ParamBMS::Load(const char* filepath) {
	if (strlen(filepath) > sizeof(pathname)) return 1;
	strcpy (pathname, filepath);

	try {
		ptree pt;
		read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

		count_sample_max_    = pt.get("SampleCount.<xmlattr>.max",   30);
		count_per_shape_min_ = pt.get("CountPerShape.<xmlattr>.min", 2);
		apex_angle_          = pt.get("Apex.<xmlattr>.angle",        60.0);
		distance_min_        = pt.get("DistanceRatio.<xmlattr>.min", 0.1);

		return 0;
	}
	catch(xml_parser_error& ex) {
		return 2;
	}
}

int ParamBMS::Save() {
	if (strlen(pathname) == 0) return 1;

	try {
		ptree pt;
		pt.put("SampleCount.<xmlattr>.max",    count_sample_max_);
		pt.put("CountPerShape.<xmlattr>.min",  count_per_shape_min_);
		pt.put("Apex.<xmlattr>.angle",         apex_angle_);
		pt.put("DistanceRatio.<xmlattr>.min",  distance_min_);

		xml_writer_settings<string> settings(' ', 4);
		write_xml(pathname, pt, std::locale(), settings);

		return 0;
	}
	catch(xml_parser_error& ex) {
		return 2;
	}
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
BuildMatchingShape::BuildMatchingShape() {
	coorsys_ = COORSYS_MIN;
	prepared_= false;
}

BuildMatchingShape::~BuildMatchingShape() {
	if (param_ != param_bak_) param_.Save();
}

/////////////////////////////////////////////////////////////////////////////
/*------------------- 功能 -------------------*/

/*------------------- 功能 -------------------*/
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/*------------------- 接口 -------------------*/
bool BuildMatchingShape::SetParamPath(const char* filepath) {
	int rslt = param_.Load(filepath);
	if (!rslt) param_bak_ = param_;
	return !rslt;
}

bool BuildMatchingShape::SetCoordinateSystem(int coorsys) {
	if (coorsys <= COORSYS_MIN || coorsys >= COORSYS_MAX) return false;
	coorsys_ = coorsys;
	return true;
}

int BuildMatchingShape::Prepare(PtBMS* ptRef) {
	if (coorsys_ <= COORSYS_MIN || coorsys_ >= COORSYS_MAX) return 1;
	if (coorsys_ == COORSYS_WCS) {
		if (!ptRef) return 2;
		ptRef_ = *ptRef;
	}

	ptSet_.clear();
	prepared_ = true;
	return 0;
}

void BuildMatchingShape::ImportSample(PtBMS& pt) {
	if (prepared_) {

	}
}

bool BuildMatchingShape::Complete() {
	if (!prepared_) return false;

	prepared_ = false;
	return true;
}

int BuildMatchingShape::Build() {
	return 0;
}

/*------------------- 接口 -------------------*/
/////////////////////////////////////////////////////////////////////////////
