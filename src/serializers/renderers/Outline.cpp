// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPango3/GlyphRenderer>
#include <osgPango3/GlyphLayer>

static bool checkOutline(const osgPango3::GlyphRendererOutline& gro) {
	const osgPango3::GlyphLayerOutline* outline =
		dynamic_cast<const osgPango3::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(outline) return outline->getOutline() != osgPango3::DEFAULT_OUTLINE;

	return false;
}

static bool readOutline(osgDB::InputStream& is, osgPango3::GlyphRendererOutline& gro) {
	osgPango3::GlyphLayerOutline* outline =
		dynamic_cast<osgPango3::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	unsigned int outlineValue;

	is >> outlineValue;

	outline->setOutline(outlineValue);

	return true;
}

static bool writeOutline(osgDB::OutputStream& os, const osgPango3::GlyphRendererOutline& gro) {
	const osgPango3::GlyphLayerOutline* outline =
		dynamic_cast<const osgPango3::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	os << outline->getOutline() << std::endl;

	return true;
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererOutline,
	new osgPango3::GlyphRendererOutline(),
	osgPango3::GlyphRendererOutline,
	"osg::Object osgPango3::GlyphRenderer osgPango3::GlyphRendererOutline"
) {
	ADD_USER_SERIALIZER(Outline);
}

