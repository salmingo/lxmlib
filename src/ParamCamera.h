/**
 * @struct ParamCamera 定义相机工作参数内存数据结构, 和xml文件访问接口
 * @version 1.0
 * @date 2020-10-13
 * @note
 * - 从xml文件中加载工作参数
 * - 将工作参数保存为xml文件
 */

#ifndef SRC_PARAMCAMERA_H_
#define SRC_PARAMCAMERA_H_

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

using std::string;
using std::vector;

/* 数据类型 */
/*!
 * @struct CameraADChannel A/D通道
 */
struct CameraADChannel {
	int index;		//< 索引
	int bitdepth;	//< 数字位数

public:
	CameraADChannel() {
		index = bitdepth = -1;
	}

	CameraADChannel &operator=(const CameraADChannel &other) {
		if (this != &other) memcpy(this, &other, sizeof(CameraADChannel));
		return *this;
	}
};
using CamADChannelVec = vector<CameraADChannel>;

/*!
 * @struct CameraReadport 读出端口
 */
struct CameraReadport {
	int index;		//< 索引
	char name[40];	//< 名称

public:
	CameraReadport() {
		index = -1;
	}

	CameraReadport &operator=(const CameraReadport &other) {
		if (this != &other) memcpy(this, &other, sizeof(CameraReadport));
		return *this;
	}
};
using CamReadportVec = vector<CameraReadport>;

/*!
 * @struct CameraReadrate 读出速度
 */
struct CameraReadrate {
	int index;		//< 索引
	char desc[10];	//< 描述

public:
	CameraReadrate() {
		index = -1;
	}

	/*!
	 * @brief 由速度构建描述
	 * @param rate 读出速度, 量纲: MHz
	 */
	void set_rate(float rate) {
		int n = int(rate + 0.001);
		if (n) sprintf (desc, "%d MHz", n);
		else sprintf (desc, "%d KHz", int(rate * 1000.0 + 0.001));
	}

	CameraReadrate &operator=(const CameraReadrate &other) {
		if (this != &other) memcpy(this, &other, sizeof(CameraReadrate));
		return *this;
	}
};
using CamReadrateVec = vector<CameraReadrate>;

/*!
 * @struct CameraReadrateSet 同一通道、端口下的读出速度集合
 */
struct CameraReadrateSet {
	int iAD, iPort;	//< AD通道和读出端口
	CamReadrateVec readrate;	//< 读出速度集合

public:
	void set_IDs(int _iAD, int _iPort) {
		iAD   = _iAD;
		iPort = _iPort;
	}

	bool is_same(int _iAD, int _iPort) const {
		return iAD == _iAD && iPort == _iPort;
	}
};
using CamReadrateSetVec = vector<CameraReadrateSet>;

struct CameraPreampGain {
	int index;	//< 索引
	float value;	//< 增益, 量纲: e-/DU

public:
	CameraPreampGain() {
		index = -1;
		value = 0.0;
	}

	CameraPreampGain &operator=(const CameraPreampGain &other) {
		if (this != &other) memcpy(this, &other, sizeof(CameraPreampGain));
		return *this;
	}
};
using CamPreampGainVec = vector<CameraPreampGain>;

struct CameraPreampGainSet {
	int iAD, iPort, iRate;	//< AD通道、读出端口、读出速度
	CamPreampGainVec preampGain;	//< 增益集合

public:
	void set_IDs(int _iAD, int _iPort, int _iRate) {
		iAD   = _iAD;
		iPort = _iPort;
		iRate = _iRate;
	}

	bool is_same(int _iAD, int _iPort, int _iRate) const {
		return iAD == _iAD && iPort == _iPort && iRate == _iRate;
	}
};
using CamPreampGainSetVec = vector<CameraPreampGainSet>;

struct CameraLineshift {
	int index;		//< 索引
	float value;	//< 速度, 量纲: 微秒/行

public:
	CameraLineshift() {
		index = -1;
		value = 0.0;
	}

	CameraLineshift &operator=(const CameraLineshift &other) {
		if (this != &other) memcpy(this, &other, sizeof(CameraLineshift));
		return *this;
	}
};
using CamLineshiftVec = vector<CameraLineshift>;

/*!
 * @struct CameraLimit 带阈值的参数
 */
struct CameraLimit {
	bool support;	//< 支持功能
	int low, high;	//< 阈值
};

struct CameraShutter {// 快门参数与状态
	bool hasMech;	//< 机械快门
	int tmopen;		//< 最短打开时间, 量纲: 毫秒
	int tmclose;	//< 最短关闭时间, 量纲: 毫秒

public:
	CameraShutter() {
		hasMech = true;
		tmopen = tmclose = 0;
	}
};

/*!
 * @struct ParamCamera
 * @brief 相机通用工作参数访问接口
 */
struct ParamCamera {
	string model;	//< 型号名称
	int sensorW, sensorH;	//< 探测器分辨率
	float pixelX, pixelY;	//< 像元尺寸, 量纲: 微米
	CamADChannelVec adc;	//< AD通道
	CamReadportVec  readport;		//< 读出端口
	CamReadrateSetVec readrate;		//< 读出速度
	CamPreampGainSetVec preampGain;	//< 前置增益
	CamLineshiftVec vsrate;	//< 行转移速度
	CameraLimit EM;		//< EM功能
	CameraLimit cooler;	//< 制冷功能
	CameraShutter shtr;	//< 快门

protected:
	string errmsg;	//< 错误提示

public:
	/*!
	 @fn bool Load(const string&)
	 * @brief
	 * 从xml文件中加载相机工作参数
	 * @param filepath  文件可访问路径
	 * @return
	 * 文件加载结果
	 */
	bool Load(const string& filepath) {
		using namespace boost::property_tree;

		ptree pt;
		try {
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);
			errmsg.clear();
		}
		catch(xml_parser_error& ex) {
			errmsg = ex.message();
			return false;
		}

		int n, i;
		char node_name[40];
		/* 基本参数 */
		ptree &node_base = pt.get_child("Standard");
		model = node_base.get("<xmlattr>.model", "");

		ptree &node_sensor = node_base.get_child("Sensor");
		sensorW = node_sensor.get("<xmlattr>.width",  1024);
		sensorH = node_sensor.get("<xmlattr>.height", 1024);

		ptree &node_pixel = node_base.get_child("PixelSize");
		pixelX  = node_pixel.get("<xmlattr>.x", 10.0);
		pixelY  = node_pixel.get("<xmlattr>.y", 10.0);

		/* A/D转换 */
		adc.clear();
		ptree &node_ad = pt.get_child("ADChannel");
		n = node_ad.get("<xmlattr>.count", 0);
		for (i = 1; i <= n; ++i) {
			sprintf (node_name, "No#%d", i);
			ptree &node = node_ad.get_child(node_name);
			CameraADChannel item;
			item.index    = node.get("<xmlattr>.index",    -1);
			item.bitdepth = node.get("<xmlattr>.bitDepth", 0);
			adc.push_back(item);
		}

		readport.clear();
		ptree &node_port = pt.get_child("ReadPort");
		n = node_port.get("<xmlattr>.count", 0);
		for (i = 1; i <= n; ++i) {
			sprintf (node_name, "No#%d", i);
			ptree &node = node_port.get_child(node_name);
			CameraReadport item;
			item.index  = node.get("<xmlattr>.index", -1);
			strcpy (item.name, node.get("<xmlattr>.name", "").c_str());
			readport.push_back(item);
		}

		readrate.clear();
		preampGain.clear();
		BOOST_FOREACH(ptree::value_type const &child, pt.get_child("")) {
			if (child.first == "ReadRate") {
				int iAD   = child.second.get("<xmlattr>.AD",   -1);
				int iPort = child.second.get("<xmlattr>.Port", -1);
				n = child.second.get("<xmlattr>.count", 0);

				CameraReadrateSet rateSet;
				rateSet.set_IDs(iAD, iPort);
				for (i = 1; i <= n; ++i) {
					sprintf (node_name, "No#%d", i);

					const ptree &node = child.second.get_child(node_name);
					CameraReadrate item;
					item.index = node.get("<xmlattr>.index", -1);
					strcpy (item.desc, node.get("<xmlattr>.desc", "").c_str());
					rateSet.readrate.push_back(item);
				}
				readrate.push_back(rateSet);
			}
			else if (child.first == "PreampGain") {
				int iAD   = child.second.get("<xmlattr>.AD",   -1);
				int iPort = child.second.get("<xmlattr>.Port", -1);
				int iRate = child.second.get("<xmlattr>.Rate", -1);
				n = child.second.get("<xmlattr>.count", 0);

				CameraPreampGainSet gainSet;
				gainSet.set_IDs(iAD, iPort, iRate);
				for (i = 1; i <= n; ++i) {
					sprintf (node_name, "No#%d", i);

					const ptree &node = child.second.get_child(node_name);
					CameraPreampGain item;
					item.index = node.get("<xmlattr>.index", -1);
					item.value = node.get("<xmlattr>.value", 0.0);
					gainSet.preampGain.push_back(item);
				}
				preampGain.push_back(gainSet);
			}
		}

		/* 行转移速度 */
		vsrate.clear();
		ptree &node_vs = pt.get_child("VSRate");
		n = node_vs.get("<xmlattr>.count", 0);
		for (i = 1; i <= n; ++i) {
			sprintf (node_name, "No#%d", i);
			ptree &node = node_vs.get_child(node_name);
			CameraLineshift item;
			item.index = node.get("<xmlattr>.index", -1);
			item.value = node.get("<xmlattr>.value", 0.0);
			vsrate.push_back(item);
		}

		/* 电子倍增 */
		ptree &node_em = pt.get_child("ElectronMultiply");
		EM.support = node_em.get("<xmlattr>.support", false);
		EM.low     = node_em.get("<xmlattr>.low",     0);
		EM.high    = node_em.get("<xmlattr>.high",    -1);

		/* 制冷 */
		ptree &node_cooler = pt.get_child("Cooler");
		cooler.support = node_cooler.get("<xmlattr>.support", true);
		cooler.low     = node_cooler.get("<xmlattr>.low",     0);
		cooler.high    = node_cooler.get("<xmlattr>.high",    -1);

		/* 机械快门 */
		ptree &node_shtr = pt.get_child("MechShutter");
		shtr.hasMech = node_shtr.get("<xmlattr>.installed", true);
		shtr.tmopen  = node_shtr.get("OpenClose.<xmlattr>.timeopen",  0);
		shtr.tmclose = node_shtr.get("OpenClose.<xmlattr>.timeclose", 0);

		return true;
	}

	/*!
	 @fn bool Save(const string&)
	 * @brief
	 * 将内存中结构体的工作参数存储为xml文件
	 * @pre
	 * - params内容被修改
	 * @param filepath 文件可写入路径
	 * @return
	 * 文件存储结果
	 */
	bool Save(const string& filepath) {
		using namespace boost::property_tree;
		ptree pt;
		int n, i;
		char node_name[40];

		/* 基本参数 */
		ptree &node_base = pt.add("Standard", "");
		node_base.add("<xmlattr>.model", model);

		ptree &node_sensor = node_base.add("Sensor", "");
		node_sensor.add("<xmlattr>.width",  sensorW);
		node_sensor.add("<xmlattr>.height", sensorH);

		ptree &node_pixel = node_base.add("PixelSize", "");
		node_pixel.add("<xmlattr>.x", pixelX);
		node_pixel.add("<xmlattr>.y", pixelX);

		/* A/D转换 */
		ptree &node_ad = pt.add("ADChannel", "");
		node_ad.add("<xmlattr>.count", n = adc.size());
		for (i = 0; i < n; ++i) {
			sprintf (node_name, "No#%d", i + 1);
			ptree &node = node_ad.add(node_name, "");
			node.add("<xmlattr>.index",    adc[i].index);
			node.add("<xmlattr>.bitDepth", adc[i].bitdepth);
		}

		ptree &node_port = pt.add("ReadPort", "");
		node_port.add("<xmlattr>.count", n = readport.size());
		for (i = 0; i < n; ++i) {
			sprintf (node_name, "No#%d", i + 1);
			ptree &node = node_port.add(node_name, "");
			node.add("<xmlattr>.index", readport[i].index);
			node.add("<xmlattr>.name",  readport[i].name);
		}

		for (CamReadrateSetVec::iterator it = readrate.begin(); it != readrate.end(); ++it) {
			ptree &node_rate = pt.add("ReadRate", "");
			node_rate.add("<xmlattr>.AD",    it->iAD);
			node_rate.add("<xmlattr>.Port",  it->iPort);
			node_rate.add("<xmlattr>.count", n = it->readrate.size());

			for (i = 0; i < n; ++i) {
				sprintf (node_name, "No#%d", i + 1);
				ptree &node = node_rate.add(node_name, "");
				node.add("<xmlattr>.index", it->readrate[i].index);
				node.add("<xmlattr>.desc", it->readrate[i].desc);
			}
		}

		for (CamPreampGainSetVec::iterator it = preampGain.begin(); it != preampGain.end(); ++it) {
			ptree &node_gain = pt.add("PreampGain", "");
			node_gain.add("<xmlattr>.AD",   it->iAD);
			node_gain.add("<xmlattr>.Port", it->iPort);
			node_gain.add("<xmlattr>.Rate", it->iRate);
			node_gain.add("<xmlattr>.count", n = it->preampGain.size());

			for (i = 0; i < n; ++i) {
				sprintf (node_name, "No#%d", i + 1);
				ptree &node = node_gain.add(node_name, "");
				node.add("<xmlattr>.index", it->preampGain[i].index);
				node.add("<xmlattr>.value", it->preampGain[i].value);
			}
		}

		/* 行转移速度 */
		ptree &node_vsrate = pt.add("VSRate", "");
		node_vsrate.add("<xmlattr>.count", n = vsrate.size());
		for (i = 0; i < n; ++i) {
			sprintf (node_name, "No#%d", i + 1);

			ptree &node = node_vsrate.add(node_name, "");
			node.add("<xmlattr>.index", vsrate[i].index);
			node.add("<xmlattr>.value", vsrate[i].value);
		}

		/* 电子倍增 */
		ptree &node_EM = pt.add("ElectronMultiply", "");
		node_EM.add("<xmlattr>.support", EM.support);
		node_EM.add("<xmlattr>.low",     EM.low);
		node_EM.add("<xmlattr>.high",    EM.high);

		/* 制冷 */
		ptree &node_cooler = pt.add("Cooler", "");
		node_cooler.add("<xmlattr>.support", cooler.support);
		node_cooler.add("<xmlattr>.low",     cooler.low);
		node_cooler.add("<xmlattr>.high",    cooler.high);

		/* 机械快门 */
		ptree &node_shtr = pt.add("MechShutter", "");
		node_shtr.add("<xmlattr>.installed", shtr.hasMech);
		node_shtr.add("OpenClose.<xmlattr>.timeopen",  shtr.tmopen);
		node_shtr.add("OpenClose.<xmlattr>.timeclose", shtr.tmclose);

		try {
			xml_writer_settings<string> settings(' ', 4);
			write_xml(filepath, pt, std::locale(), settings);
			errmsg.clear();
			return true;
		}
		catch(xml_parser_error& ex) {
			errmsg = ex.message();
			return false;
		}
	}

	/*!
	 * @brief 获得index指向的地址
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraADChannel *GetADChannel(int index) const {
		CamADChannelVec::const_iterator it;
		for (it = adc.begin(); it != adc.end() && index != it->index; ++it);
		return it == adc.end() ? NULL : &(*it);
	}

	/*!
	 * @brief 获得index指向的地址
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraReadport *GetReadport(int index) const {
		CamReadportVec::const_iterator it;
		for (it = readport.begin(); it != readport.end() && index != it->index; ++it);
		return it == readport.end() ? NULL : &(*it);
	}

	/*!
	 * @brief 获得指定的读出速度集合
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraReadrateSet *GetReadrateSet(int iAD, int iPort) const {
		CamReadrateSetVec::const_iterator it;
		for (it = readrate.begin(); it != readrate.end() && !it->is_same(iAD, iPort); ++it);
		return it == readrate.end() ? NULL : &(*it);
	}

	/*!
	 * @brief 获得index指向的地址
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraReadrate *GetReadrate(int iAD, int iPort, int index) const {
		const CameraReadrateSet *rateSet = GetReadrateSet(iAD, iPort);
		if (rateSet) {
			const CamReadrateVec &rates = rateSet->readrate;
			CamReadrateVec::const_iterator it;
			for (it = rates.begin(); !(it == rates.end() || index == it->index); ++it);
			return it == rates.end() ? NULL : &(*it);
		}
		return NULL;
	}

	/*!
	 * @brief 获得指定的前置增益集合
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraPreampGainSet *GetPreampGainSet(int iAD, int iPort, int iRate) const {
		CamPreampGainSetVec::const_iterator it;
		for (it = preampGain.begin(); !(it == preampGain.end() || it->is_same(iAD, iPort, iRate)); ++it);
		return it == preampGain.end() ? NULL : &(*it);
	}

	/*!
	 * @brief 获得index指向的地址
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraPreampGain *GetPreampGain(int iAD, int iPort, int iRate, int index) const {
		const CameraPreampGainSet *gainSet = GetPreampGainSet(iAD, iPort, iRate);
		if (gainSet) {
			const CamPreampGainVec &gains = gainSet->preampGain;
			CamPreampGainVec::const_iterator it;
			for (it = gains.begin(); !(it == gains.end() || index == it->index); ++it);
			return it == gains.end() ? NULL : &(*it);
		}
		return NULL;
	}

	/*!
	 * @brief 获得index指向的地址
	 * @return
	 * - 成功: 地址
	 * - 失败: NULL
	 */
	const CameraLineshift *GetLineshift(int index) const {
		CamLineshiftVec::const_iterator it;
		for (it = vsrate.begin(); it != vsrate.end() && index != it->index; ++it);
		return it == vsrate.end() ? NULL : &(*it);
	}
};

#endif /* SRC_PARAMCAMERA_H_ */
