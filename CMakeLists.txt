PROJECT(osgPango)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/etc/")

SET(OSGPANGO_ROOT "${CMAKE_SOURCE_DIR}")

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib)
ENDIF()

# Use a debug postfix to distinguish build products. Mostly important on
# Windows, because linking a debug library into a release executable (or
# vice-versa, or just mixing C++ runtime versions) on Windows will lead
# to crashes if the libraries use the C++ runtime.
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Add a postfix, usually d on windows")

# On GCC, we need to set these compiler flags.
IF(NOT WIN32)
	SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused")
ENDIF(NOT WIN32)

IF(NOT WIN32)
	INCLUDE(FindPkgConfig)
	PKG_CHECK_MODULES(CAIRO REQUIRED cairo)
	PKG_CHECK_MODULES(PANGO REQUIRED pango)
	PKG_CHECK_MODULES(PANGOCAIRO REQUIRED pangocairo)
ELSE(NOT WIN32)
	FIND_PACKAGE(Win32GTKBundle)
ENDIF(NOT WIN32)

FIND_PACKAGE(OpenThreads)
FIND_PACKAGE(osg)
FIND_PACKAGE(osgDB)
FIND_PACKAGE(osgGA)
FIND_PACKAGE(osgUtil)
FIND_PACKAGE(osgText)
FIND_PACKAGE(osgViewer)
FIND_PACKAGE(osgCairo)

SET(SRC_FILES
	"src/Context.cpp"
	"src/DistanceFieldText.cpp"
	"src/Glyph.cpp"
	"src/GlyphLayerInterface.cpp"
	"src/GlyphLayer.cpp"
	"src/GlyphRenderer.cpp"
	"src/ShaderGenerator.cpp"
	"src/ShaderManager.cpp"
	"src/Text.cpp"
	"src/TextTransform.cpp"
	"src/Util.cpp"
	"src/VerticalText.cpp"
)

SET(SRC_LAYER_FILES
	"src/layers/Bitmap.cpp"
	"src/layers/Bevel.cpp"
	"src/layers/DistanceField.cpp"
	"src/layers/Outline.cpp"
	"src/layers/ShadowBlur.cpp"
	"src/layers/ShadowInset.cpp"
	"src/layers/Shadow.cpp"
)

SET(SRC_RENDERER_FILES
	"src/renderers/Default.cpp"
	"src/renderers/DistanceField.cpp"
	"src/renderers/Outline.cpp"
	"src/renderers/ShadowBlur.cpp"
	"src/renderers/ShadowInset.cpp"
	"src/renderers/Shadow.cpp"
)

SET(SRC_SERIALIZER_FILES
	"src/serializers/Glyph.cpp"
	"src/serializers/GlyphRenderer.cpp"
	"src/serializers/LibraryWrapper.cpp"
	"src/serializers/renderers/Default.cpp"
	"src/serializers/renderers/Outline.cpp"
	"src/serializers/renderers/Shadow.cpp"
)

SET(HEADER_DIR ${osgPango_SOURCE_DIR}/include/osgPango3)

SET(HEADER_FILES
	"${HEADER_DIR}/Context"
	"${HEADER_DIR}/Export"
	"${HEADER_DIR}/Glyph"
	"${HEADER_DIR}/GlyphLayerInterface"
	"${HEADER_DIR}/GlyphLayer"
	"${HEADER_DIR}/GlyphRenderer"
	"${HEADER_DIR}/Serialize"
	"${HEADER_DIR}/ShaderGenerator"
	"${HEADER_DIR}/ShaderManager"
	"${HEADER_DIR}/String"
	"${HEADER_DIR}/Text"
	"${HEADER_DIR}/TextTransform"
	"${HEADER_DIR}/Util"
	"${HEADER_DIR}/Version"
	"${HEADER_DIR}/VerticalText"
)

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})
SOURCE_GROUP("Source Files\\Layers" FILES ${SRC_LAYER_FILES})
SOURCE_GROUP("Source Files\\Renderers" FILES ${SRC_RENDERER_FILES})
SOURCE_GROUP("Source Files\\Serializers" FILES ${SRC_SERIALIZER_FILES})

INCLUDE_DIRECTORIES(include
        ${OSG_INCLUDE_DIR}
        ${OSGCAIRO_INCLUDE_DIR}
	${CAIRO_INCLUDE_DIRS}
	${PANGOCAIRO_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${GLIBCONFIG_INCLUDE_DIRS}
	${FONTCONFIG_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
	${OSG_LIB_DIR}
	${OSGCAIRO_LIB_DIR}
	${PANGOCAIRO_LIBRARY_DIRS}
)

SET(OSGPANGO_LIBRARY_NAME "osgPango" CACHE STRING "base name for osgPango library")

ADD_LIBRARY(${OSGPANGO_LIBRARY_NAME} SHARED
	${SRC_FILES}
	${SRC_LAYER_FILES}
	${SRC_RENDERER_FILES}
	${SRC_SERIALIZER_FILES}
	${HEADER_FILES}
)

# Add debug postfix to OSG libraries so we link to the right ones in debug.
# Cairo, pango, pangocairo and the gtk libs are C-only libraries so the same
# one (release) can be linked to both debug and release without problems.
TARGET_LINK_LIBRARIES(${OSGPANGO_LIBRARY_NAME}
        ${OSG_LIBRARY} ${OSGUTIL_LIBRARY} ${OSGDB_LIBRARY} ${OSGGA_LIBRARY} ${OSGTEXT_LIBRARY} ${OSGVIEWER_LIBRARY} ${OPENTHREADS_LIBRARY}
        ${OSGCAIRO_LIBRARY}
        debug     cairo
        optimized cairo
	debug     pango-1.0
	optimized pango-1.0
	debug     pangocairo-1.0
	optimized pangocairo-1.0
	debug     gobject-2.0
	optimized gobject-2.0
	debug     glib-2.0
	optimized glib-2.0
	debug     fontconfig
	optimized fontconfig
)

SET_TARGET_PROPERTIES(${OSGPANGO_LIBRARY_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OSGPANGO_ROOT}/lib)

# The archive/runtime part is so that the lib/dll pair goes into lib and bin
# respectively on Windows.
INSTALL(TARGETS ${OSGPANGO_LIBRARY_NAME}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

FOREACH(INCLUDEFILE ${HEADER_FILES})
	FILE(RELATIVE_PATH REL_INCLUDEFILE ${osgPango_SOURCE_DIR}/include/osgPango ${INCLUDEFILE})
	GET_FILENAME_COMPONENT(REL_INCLUDE_PATH ${REL_INCLUDEFILE} PATH)
	INSTALL(
		FILES ${INCLUDEFILE}
		DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${OSGPANGO_LIBRARY_NAME}/${REL_INCLUDE_PATH}
	)
ENDFOREACH(INCLUDEFILE)

ADD_SUBDIRECTORY(applications/osgpango-gr-cache)

ADD_SUBDIRECTORY(examples/osgpangoanimation)
ADD_SUBDIRECTORY(examples/osgpangocomplexrenderer)
ADD_SUBDIRECTORY(examples/osgpangodistancefield)
ADD_SUBDIRECTORY(examples/osgpangofonts)
ADD_SUBDIRECTORY(examples/osgpangoglsl)
ADD_SUBDIRECTORY(examples/osgpangosimplerenderer)
ADD_SUBDIRECTORY(examples/osgpangoupdatecb)
ADD_SUBDIRECTORY(examples/osgpangovertical)
ADD_SUBDIRECTORY(examples/osgpangoviewer)
#ADD_SUBDIRECTORY(examples/osgpangoguiviewer)
#ADD_SUBDIRECTORY(examples/osgpangothreaded)

CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/etc/uninstall.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
	IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
)

# Packaging information.
SET(CPACK_GENERATOR TGZ)
SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_IGNORE_FILES
	"~$"
	".*.svn"
	".*build/"
	".*debug/"
	".*release/"
)

SET(OSGPANGO_VERSION "2.2.0")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "osgPango-${OSGPANGO_VERSION}")

INCLUDE(CPack)
