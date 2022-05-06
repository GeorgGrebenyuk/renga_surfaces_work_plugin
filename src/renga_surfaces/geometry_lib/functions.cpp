#include "functions.h"
using namespace tinyxml2;
xml2iges::xml2iges(const char* path_to_landxml, std::vector<float> offset_parameters)
{
	this->landxml_path = path_to_landxml;
	this->read_xml();
}
void xml2iges::read_xml()
{
	XMLDocument doc;
	doc.LoadFile("dream.xml");
}