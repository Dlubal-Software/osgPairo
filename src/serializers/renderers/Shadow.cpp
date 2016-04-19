// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango3/GlyphRenderer>
#include <osgPango3/GlyphLayer>
#include <osgPango3/Serialize>

OFFSET_CHECK (osgPango3::GlyphRendererShadow, 1)
OFFSET_READ  (osgPango3::GlyphRendererShadow, 1)
OFFSET_WRITE (osgPango3::GlyphRendererShadow, 1)

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererShadow,
	new osgPango3::GlyphRendererShadow(),
	osgPango3::GlyphRendererShadow,
	"osg::Object osgPango3::GlyphRenderer osgPango3::GlyphRendererShadow"
) {
	ADD_USER_SERIALIZER(Offset);
}

