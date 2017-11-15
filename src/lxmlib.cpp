/*
 名称 : lxmlib.cpp
 作者 : 卢晓猛
 版本 : 0.1
 版权 :
 描述 :
 - 归档自开发常用软件
 - 测试自开发常用软件
 */

#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "GLog.h"
//#include "globaldef.h"
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <dirent.h>
#include "WCSTNX.h"

using std::vector;
using std::string;

GLog _gLog(stdout);

int filter_dir(const struct dirent *ent) {
	if (ent->d_type != DT_REG || ent->d_name[0] == '.') return 0;
	return 1;
}

int main(int argc, char **argv) {
	boost::asio::io_service ios;
	boost::asio::signal_set signals(ios, SIGINT, SIGTERM);  // interrupt signal
	signals.async_wait(boost::bind(&boost::asio::io_service::stop, &ios));

///////////////////////////////////////////////////////////////////////////////
// 功能测试区
	if (argc < 3) {
		printf("Usage:\n\t lxmlib path_of_acc path_of_fits\n");
//		printf("Usage:\n\t lxmlib path_of_acc x y\n");
		return -1;
	}
	string fileacc  = argv[1];
	string filefits = argv[2];
//	double x = atof(argv[2]);
//	double y = atof(argv[3]);
//	double ra, dec;
	WCSTNX wcstnx;
	if (!wcstnx.LoadText(fileacc.c_str())) {
		printf("failed to load calibration file\n");
		return -2;
	}
	_gLog.Write("writting file<%s>", filefits.c_str());
	if (wcstnx.WriteImage(filefits.c_str())) {
		printf("failed to write fits file\n");
		return -3;
	}
	_gLog.Write("complete");

//	wcstnx.XY2WCS(x, y, ra, dec);
//	printf("%7.2f %7.2f ==> %9.5f %9.5f\n", x, y, ra * R2D, dec * R2D);

/*
	if (argc < 4) {
		printf("Usage:\n\t lxmlib filepath x y\n");
		return -1;
	}
	namespace fs = boost::filesystem;
	fs::path pathroot = argv[1];
	double x = atof(argv[2]);
	double y = atof(argv[3]);
	double ra, dec;
	double rotmin(AMAX), rotmax(-AMAX), rot;
	fs::path filepath, filename;
	fs::path extacc(".acc");
	fs::path extfit(".fit");
	int n;
	struct dirent **namelist;
	WCSTNX wcstnx;
	FILE *rslt = fopen("result.txt", "w");
	if (!rslt) {
		_gLog.Write("failed to create result.txt");
		return -2;
	}

	n = scandir(pathroot.c_str(), &namelist, filter_dir, alphasort);
	for (int i = 0; i < n; ++i) {
		filename = namelist[i]->d_name;
		free(namelist[i]);
		if (filename.extension().compare(extacc)) continue;
		filepath = pathroot;
		filepath /= filename;
		if (!wcstnx.LoadText(filepath.c_str())) {
			_gLog.Write(LOG_FAULT, NULL, "failed to load parameters from %s", filepath.c_str());
		}
		else if (!wcstnx.XY2WCS(x, y, ra, dec)) {
			rot = wcstnx.GetParam()->rotation.x;
			if (rot < rotmin) rotmin = rot;
			if (rot > rotmax) rotmax = rot;
			filename.replace_extension(extfit);
			printf("%9.5f %9.5f %s %5.1f %5.1f\n", ra * R2D, dec * R2D, filename.c_str(),
					rot * R2D, wcstnx.GetParam()->rotation.y * R2D);
			fprintf(rslt, "%9.5f %9.5f %s %5.1f %5.1f\n", ra * R2D, dec * R2D, filename.c_str(),
					rot * R2D, wcstnx.GetParam()->rotation.y * R2D);
		}
	}
	if (n) free(namelist);
	if (n) {
		printf("Rotation Minimum = %.1f\n", rotmin * R2D);
		printf("Rotation Maximum = %.1f\n", rotmax * R2D);
	}

	fclose(rslt);
*/
//////////////////////////////////////////////////////////////////////////////

	ios.run();

	return 0;
}
