// -*-c++-*- osgCairo - Copyright (C) 2011 osgCairo Development Team
// $Id$

#include <osgCairo/Notify>
#include <osgCairo/ReadFile>

namespace osgCairo {

Image* readImageFile(const std::string& path, osgDB::ReaderWriter::Options* options) {
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(path, options);

	if(!image) return 0;

	osg::ref_ptr<Image> cairoImage = new Image();

	if(cairoImage->allocateSurface(image.get()))
        {
            return cairoImage.release();
        }

	return 0;
}

}

