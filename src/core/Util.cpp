// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Math>
#include <osgDB/FileUtils>
#include <osgPairo/Util>

#include "cairocks.h"

namespace osgPairo {

std::string getFilePath(const std::string& filename) {
	osgDB::FilePathList  path;
	osgDB::FilePathList& paths = osgDB::getDataFilePathList();

	char* fp = getenv("OSGPANGO_FILE_PATH");

	osgDB::convertStringPathIntoFilePathList(fp ? fp : ".", path);

	for(osgDB::FilePathList::iterator i = path.begin(); i != path.end(); i++) paths.push_back(*i);

	return osgDB::findDataFile(filename);
}

void roundVec3(osg::Vec3& vec) {
	vec[0] = osg::round(vec[0]);
	vec[1] = osg::round(vec[1]);
	vec[2] = osg::round(vec[2]);
}

bool roundedRectangle(
	cairo_t*       c,
	double         x,
	double         y,
	double         width,
	double         height,
	double         radius,
	const Corners& corners
) {
	return cairorocks::cairocks_rounded_rectangle(c, x, y, width, height, radius, corners.corners) != FALSE;
}

bool roundedRectangleApply(
	cairo_t*       c,
	double         x,
	double         y,
	double         width,
	double         height,
	double         radius,
	const Corners& corners
) {
	return cairorocks::cairocks_rounded_rectangle_apply(c, x, y, width, height, radius, corners.corners) != FALSE;
}

bool gaussianBlur(cairo_surface_t* surface, double radius, double deviation) {
	return cairorocks::cairocks_gaussian_blur(surface, radius, deviation) != FALSE;
}

bool mapPathOnto(cairo_t* c, cairo_path_t* path) {
	return cairorocks::cairocks_map_path_onto(c, path) != FALSE;
}

cairo_surface_t* createEmbossedSurface(
	cairo_surface_t* surface,
	double           azimuth,
	double           elevation,
	double           height,
	double           ambient,
	double           diffuse
) {
	return cairorocks::cairocks_emboss_create(surface, azimuth, elevation, height, ambient, diffuse);
}

cairo_surface_t* createDistanceField(
	cairo_surface_t* surface,
	int              scan_size,
	int              block_size
) {
	return cairorocks::cairocks_distance_field_create(surface, scan_size, block_size);
}

}
