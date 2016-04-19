// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango3/GlyphRenderer>

static bool checkTextureSize(const osgPango3::GlyphRenderer& gr) {
}

static bool readTextureSize(osgDB::InputStream& is, osgPango3::GlyphRenderer& gr) {
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPango3::GlyphRenderer& gr) {
}

REGISTER_OBJECT_WRAPPER(
	osgPango3_GlyphLayerInterface_Offset,
	0,
	osgPango3::GlyphRenderer,
	"osg::Object osgPango3::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);

	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

