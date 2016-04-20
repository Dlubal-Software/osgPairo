// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgPairo/GlyphRenderer>
#include <osgPairo/GlyphLayer>

static bool checkOutline(const osgPairo::GlyphRendererOutline& gro) {
	const osgPairo::GlyphLayerOutline* outline =
		dynamic_cast<const osgPairo::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(outline) return outline->getOutline() != osgPairo::DEFAULT_OUTLINE;

	return false;
}

static bool readOutline(osgDB::InputStream& is, osgPairo::GlyphRendererOutline& gro) {
	osgPairo::GlyphLayerOutline* outline =
		dynamic_cast<osgPairo::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	unsigned int outlineValue;

	is >> outlineValue;

	outline->setOutline(outlineValue);

	return true;
}

static bool writeOutline(osgDB::OutputStream& os, const osgPairo::GlyphRendererOutline& gro) {
	const osgPairo::GlyphLayerOutline* outline =
		dynamic_cast<const osgPairo::GlyphLayerOutline*>(gro.getLayer(1))
	;

	if(!outline) return false;

	os << outline->getOutline() << std::endl;

	return true;
}

REGISTER_OBJECT_WRAPPER(
	osgPango_GlyphRendererOutline,
	new osgPairo::GlyphRendererOutline(),
	osgPairo::GlyphRendererOutline,
	"osg::Object osgPairo::GlyphRenderer osgPairo::GlyphRendererOutline"
) {
	ADD_USER_SERIALIZER(Outline);
}

