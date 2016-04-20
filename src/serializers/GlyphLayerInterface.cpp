// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPairo/GlyphRenderer>

static bool checkTextureSize(const osgPairo::GlyphRenderer& gr) {
}

static bool readTextureSize(osgDB::InputStream& is, osgPairo::GlyphRenderer& gr) {
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPairo::GlyphRenderer& gr) {
}

REGISTER_OBJECT_WRAPPER(
	osgPairo_GlyphLayerInterface_Offset,
	0,
	osgPairo::GlyphRenderer,
	"osg::Object osgPairo::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);

	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

