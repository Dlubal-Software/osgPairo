// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgPango3/GlyphRenderer>

// ------------------------------------------------------------------------------------ TextureSize
static bool checkTextureSize(const osgPango3::GlyphRenderer& gr) {
	const osg::Vec2s& ts = gr.getTextureSize();

	return (
		ts.x() != osgPango3::DEFAULT_TEXTURE_WIDTH ||
		ts.y() != osgPango3::DEFAULT_TEXTURE_HEIGHT
	);
}

static bool readTextureSize(osgDB::InputStream& is, osgPango3::GlyphRenderer& gr) {
	osg::Vec2s size;

	is >> size;

	gr.setTextureSize(size);

	return true;
}

static bool writeTextureSize(osgDB::OutputStream& os, const osgPango3::GlyphRenderer& gr) {
	os << gr.getTextureSize() << std::endl;

	return true;
}

// ---------------------------------------------------------------------------------- MinFilterMode
static bool checkMinFilterMode(const osgPango3::GlyphRenderer& gr) {
	return gr.getMinFilterMode() != osg::Texture::LINEAR;
}

static bool readMinFilterMode(osgDB::InputStream& is, osgPango3::GlyphRenderer& gr) {
	DEF_GLENUM(mode);

	is >> mode;

	gr.setMinFilterMode(static_cast<osg::Texture::FilterMode>(mode.get()));

	return true;
}

static bool writeMinFilterMode(osgDB::OutputStream& os, const osgPango3::GlyphRenderer& gr) {
	os << GLENUM(gr.getMinFilterMode()) << std::endl;

	return true;
}

// ------------------------------------------------------------------------------ FontGlyphCacheMap
static bool checkFontGlyphCacheMap(const osgPango3::GlyphRenderer& gr) {
	const osgPango3::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	return fgcm.size() != 0;
}

static bool readFontGlyphCacheMap(osgDB::InputStream& is, osgPango3::GlyphRenderer& gr) {
	osgPango3::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	unsigned int size = is.readSize();

	is >> is.BEGIN_BRACKET;

	for(unsigned int i = 0; i < size; i++) {
                osg::ref_ptr<osg::Object> obj = is.readObject();
		osgPango3::GlyphCache* gc = dynamic_cast<osgPango3::GlyphCache*>(obj.get());

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

static bool writeFontGlyphCacheMap(osgDB::OutputStream& os, const osgPango3::GlyphRenderer& gr) {
	const osgPango3::GlyphRenderer::FontGlyphCacheMap& fgcm = gr.getGlyphCaches();

	os.writeSize(fgcm.size());

	os << os.BEGIN_BRACKET << std::endl;

	for(
		osgPango3::GlyphRenderer::FontGlyphCacheMap::const_iterator i = fgcm.begin();
		i != fgcm.end();
		i++
	) {
		os.writeObject(i->second.get());
	}

	os << os.END_BRACKET << std::endl;

	return true;
}

REGISTER_OBJECT_WRAPPER(
	osgPango3_GlyphRenderer,
	0,
	osgPango3::GlyphRenderer,
	"osg::Object osgPango3::GlyphRenderer"
) {
	ADD_UINT_SERIALIZER(PixelSpacing, 1);

	ADD_USER_SERIALIZER(TextureSize);
	ADD_USER_SERIALIZER(MinFilterMode);
	ADD_USER_SERIALIZER(FontGlyphCacheMap);
}

