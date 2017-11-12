/*!
 * @file parameter.h 使用XML文件格式管理配置参数
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <string>
#include <vector>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

using std::string;

enum MOUNT_TYPE {// 转台类型
	MOUNT_GWAC,		//< GWAC类型转台
	MOUNT_NORMAL,	//< 通用型望远镜
	MOUNT_LAST		//< 占位符
};

struct ObsSite {// 测站信息
	string group_id;		//< 在网络系统中的组标志
	MOUNT_TYPE  mntype;	//< 转台类型. 2017-09-25
	string name;		//< 测站名称
	double lgt;		//< 地理经度, 东经为正, 量纲: 角度
	double lat;		//< 地理纬度, 北纬为正, 量纲: 角度
	double alt;		//< 海拔高度, 量纲: 米
};
typedef std::vector<ObsSite> SiteVec;

struct param_config {// 软件配置参数
	uint16_t portClient;		//< 面向客户端网络服务端口
	uint16_t portDB;			//< 面向数据库网络服务端口
	uint16_t portMount;		//< 面向GWAC转台网络服务端口
	uint16_t portFocus;		//< 面向GWAC调焦器网络服务端口
	uint16_t portTele;		//< 面向通用望远镜网络服务端口
	uint16_t portCamera;		//< 面向相机网络服务端口
	uint16_t portAnnex;		//< 面向GWAC定制相机配套软件网络服务端口

	bool enableNTP;		//< NTP启用标志
	string hostNTP;		//< NTP服务器IP地址
	int  maxDiffNTP;		//< 采用自动校正时钟策略时, 本机时钟与NTP时钟所允许的最大偏差, 量纲: 毫秒

	SiteVec sites;		//< 测站位置信息

public:
	/*!
	 * @brief 初始化文件filepath, 存储缺省配置参数
	 * @param filepath 文件路径
	 */
	void InitFile(const string& filepath) {
		using namespace boost::posix_time;
		using boost::property_tree::ptree;

		ptree pt;

		pt.add("version", "0.7");
		pt.add("date", to_iso_string(second_clock::universal_time()));

		ptree& node1 = pt.add("Server", "");
		node1.add("Client",    portClient = 4010);
		node1.add("Database",  portDB     = 4011);
		node1.add("Mount",     portMount  = 4012);
		node1.add("Focus",     portFocus  = 4013);
		node1.add("Telescope", portTele   = 4014);
		node1.add("Camera",    portCamera = 4015);
		node1.add("Annex",     portAnnex  = 4016);

		pt.add("NTP.<xmlattr>.Enable",  enableNTP = true);
		pt.add("NTP.<xmlattr>.IP",      hostNTP = "172.28.1.3");
		pt.add("NTP.<xmlattr>.MaxDiff", maxDiffNTP = 5);

		ptree& node2 = pt.add("Observatory", "");
		ObsSite site;
		node2.add("group_id",    site.group_id = "001");
		node2.add("device_type", "GWAC");
		node2.add("name",      site.name     = "Xinglong");
		node2.add("longitude", site.lgt      = 117.57454166666667);
		node2.add("latitude",  site.lat      = 40.395933333333333);
		node2.add("altitude",  site.alt      = 900);
		site.mntype = MOUNT_GWAC;
		sites.push_back(site);

		ptree& node3 = pt.add("Observatory", "");
		ObsSite site1;
		node3.add("group_id",    site1.group_id = "002");
		node3.add("device_type", "Normal");
		node3.add("name",      site1.name     = "Xinglong");
		node3.add("longitude", site1.lgt      = 117.57454166666667);
		node3.add("latitude",  site1.lat      = 40.395933333333333);
		node3.add("altitude",  site1.alt      = 900);
		site.mntype = MOUNT_NORMAL;
		sites.push_back(site1);

		boost::property_tree::xml_writer_settings<string> settings(' ', 4);
		write_xml(filepath, pt, std::locale(), settings);
	}

	/*!
	 * @brief 从文件filepath加载配置参数
	 * @param filepath 文件路径
	 */
	void LoadFile(const string& filepath) {
		try {
			using boost::property_tree::ptree;

			string value;
			ptree pt;
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

			portClient = pt.get("Server.Client",    4010);
			portDB     = pt.get("Server.Database",  4011);
			portMount  = pt.get("Server.Mount",     4012);
			portFocus  = pt.get("Server.Focus",     4013);
			portTele   = pt.get("Server.Telescope", 4014);
			portCamera = pt.get("Server.Camera",    4015);
			portAnnex  = pt.get("Server.Annex",     4016);

			enableNTP  = pt.get("NTP.<xmlattr>.Enable", false);
			hostNTP    = pt.get("NTP.<xmlattr>.IP",     "127.0.0.1");
			maxDiffNTP = pt.get("NTP.<xmlattr>.MaxDiff", 5);

			BOOST_FOREACH(ptree::value_type const &child, pt.get_child("")) {
				if (boost::iequals(child.first, "Observatory")) {
					ObsSite site;
					site.group_id = child.second.get("group_id",     "");
					value         = child.second.get("device_type", "GWAC");
					site.name     = child.second.get("name",         "");
					site.lgt      = child.second.get("longitude",   0.0);
					site.lat      = child.second.get("latitude",    0.0);
					site.alt      = child.second.get("altitude",  100.0);
					if (boost::iequals(value, "GWAC")) site.mntype = MOUNT_GWAC;
					else site.mntype = MOUNT_NORMAL;
					sites.push_back(site);
				}
			}
		}
		catch(boost::property_tree::xml_parser_error &ex) {
			InitFile(filepath);
		}
	}

	/*!
	 * @brief 从配置项中查找group_id对应的设备类型
	 * @param group_id  GWAC组标志
	 * @param devtype   设备类型. 可接受数值: {GWAC, GFT}
	 */
	MOUNT_TYPE GetDevtype(const string group_id) {
		SiteVec::iterator it;
		for (it = sites.begin(); it != sites.end() && !boost::iequals(group_id, (*it).group_id); ++it);
		return ((it != sites.end()) ? (*it).mntype : MOUNT_LAST);
	}

	/*!
	 * @brief 从配置项中查找group_id对应的测站地理位置
	 * @param group_id  GWAC组标志
	 * @param lgt       地理经度, 东经为正, 量纲: 角度
	 * @param lat		地理纬度, 北纬为正, 量纲: 角度
	 * @param alt		海拔高度, 量纲: 米
	 */
	void GetGeosite(const string group_id, string& name, double &lgt, double &lat, double &alt) {
		SiteVec::iterator it;

		lgt = lat = alt = 0.0;
		for (it = sites.begin(); it != sites.end() && !boost::iequals(group_id, (*it).group_id); ++it);
		if (it != sites.end()) {
			name = (*it).name;
			lgt  = (*it).lgt;
			lat  = (*it).lat;
			alt  = (*it).alt;
		}
		else name = "";
	}
};

#endif // PARAMETER_H_
