// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPairo/GlyphRenderer>
#include <osgPairo/GlyphLayer>
#include <osgPairo/Serialize>

OFFSET_CHECK (osgPairo::GlyphRendererShadow, 1)
OFFSET_READ  (osgPairo::GlyphRendererShadow, 1)
OFFSET_WRITE (osgPairo::GlyphRendererShadow, 1)

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererShadow,
	new osgPairo::GlyphRendererShadow(),
	osgPairo::GlyphRendererShadow,
	"osg::Object osgPairo::GlyphRenderer osgPairo::GlyphRendererShadow"
) {
	ADD_USER_SERIALIZER(Offset);
}

