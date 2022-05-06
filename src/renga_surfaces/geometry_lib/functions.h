#pragma once
#include "tinyxml2.h"
#include <vector>
#include <string>
class xml2iges {
public:
	xml2iges(const char* path_to_landxml, std::vector<float> offset_parameters);
	std::vector<float> offset_parameters;
	std::string file_path;
private:
	std::string landxml_path;
	void recalc_points(float* x, float* y, float* z);
	void read_xml();
	void write_iges();
};