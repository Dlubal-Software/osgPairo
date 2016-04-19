// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango3/GlyphRenderer>

REGISTER_OBJECT_WRAPPER(
	osgPango3_GlyphRendererDefault,
	new osgPango3::GlyphRendererDefault(),
	osgPango3::GlyphRendererDefault,
	"osg::Object osgPango3::GlyphRenderer osgPango3::GlyphRendererDefault"
) {
}

