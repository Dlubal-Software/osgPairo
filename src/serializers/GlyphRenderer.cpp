// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPairo/GlyphRenderer>

// ------------------------------------------------------------------------------------ TextureSize
static bool checkTextureSize(const osgPairo::GlyphRenderer& gr) {
	const osg::Vec2s& ts = gr.getTextureSize();

	return (
		ts.x() != osgPairo::DEFAULT_TEXTURE_WIDTH ||
		ts.y() != osgPairo::DEFAULT_TEXTURE_HEIGHT
	);
}

static bool readTextureSize(osgDB::InputStream& is, osgPairo::GlyphRenderer& gr) {
	osg::Vec2s size;

	is >> size;

	gr.setTextureSize(size);

	return true;
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPairo::GlyphRenderer& gr) {
	os << gr.getTextureSize() << std::endl;

	return true;
}

// ---------------------------------------------------------------------------------- MinFilterMode
static bool checkMinFilterMode(const osgPairo::GlyphRenderer& gr) {
	return gr.getMinFilterMode() != osg::Texture::LINEAR;
}

static bool readMinFilterMode(osgDB::InputStream& is, osgPairo::GlyphRenderer& gr) {
	DEF_GLENUM(mode);

	is >> mode;

	gr.setMinFilterMode(static_cast<osg::Texture::FilterMode>(mode.get()));

	return true;
}

static bool writeMinFilterMode(osgDB::OutputStream& os, const osgPairo::GlyphRenderer& gr) {
	os << GLENUM(gr.getMinFilterMode()) << std::endl;

	return true;
}

// ------------------------------------------------------------------------------ FontGlyphCacheMap
static bool checkFontGlyphCacheMap(const osgPairo::GlyphRenderer& gr) {
	const osgPairo::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	return fgcm.size() != 0;
}

static bool readFontGlyphCacheMap(osgDB::InputStream& is, osgPairo::GlyphRenderer& gr) {
	osgPairo::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	unsigned int size = is.readSize();

	is >> is.BEGIN_BRACKET;

	for(unsigned int i = 0; i < size; i++) {
                osg::ref_ptr<osg::Object> obj = is.readObject();
		osgPairo::GlyphCache* gc = dynamic_cast<osgPairo::GlyphCache*>(obj.get());

		if(!gc) {
			OSG_WARN << "SHIT" << std::endl;

			return false;
		}

		gc->setGlyphRenderer(&gr);

		fgcm[gc->getHash()] = gc;
	}

	is >> is.END_BRACKET;

	return true;
}

static bool writeFontGlyphCacheMap(osgDB::OutputStream& os, const osgPairo::GlyphRenderer& gr) {
	const osgPairo::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	os.writeSize(fgcm.size());

	os << os.BEGIN_BRACKET << std::endl;

	for(
		osgPairo::GlyphRenderer::FontGlyphCacheMap::const_iterator i = fgcm.begin();
		i != fgcm.end();
		i++
	) {
		os.writeObject(i->second.get());
	}

	os << os.END_BRACKET << std::endl;

	return true;
}

REGISTER_OBJECT_WRAPPER(
	osgPairo_GlyphRenderer,
	0,
	osgPairo::GlyphRenderer,
	"osg::Object osgPairo::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);

	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

