// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPairo/GlyphRenderer>

REGISTER_OBJECT_WRAPPER(
	osgPairo_GlyphRendererDefault,
	new osgPairo::GlyphRendererDefault(),
	osgPairo::GlyphRendererDefault,
	"osg::Object osgPairo::GlyphRenderer osgPairo::GlyphRendererDefault"
) {
}

