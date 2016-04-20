// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgDB/Registry>

USE_SERIALIZER_WRAPPER(osgPairo_GlyphRenderer)
USE_SERIALIZER_WRAPPER(osgPairo_GlyphRendererDefault)
USE_SERIALIZER_WRAPPER(osgPairo_GlyphCache)

extern "C" void wrapper_serializer_library_osgPairo(void) {
	// When is this called?
	OSG_NOTICE << "wrapper_serializer_library_osgPairo" << std::endl;
}

