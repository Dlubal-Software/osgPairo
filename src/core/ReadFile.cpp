// -*-c++-*- osgPairo - Copyright (C) 2011 osgPairo Development Team
// $Id$

#include <osgPairo/Notify>
#include <osgPairo/ReadFile>

osgPairo::Image* osgPairo::readImageFile(const std::string& path, osgDB::ReaderWriter::Options* options)
{
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(path, options);

	if(!image) return 0;

	osg::ref_ptr<osgPairo::Image> cairoImage = new osgPairo::Image();

	if(cairoImage->allocateSurface(image.get()))
        {
            return cairoImage.release();
        }

	return 0;
}

