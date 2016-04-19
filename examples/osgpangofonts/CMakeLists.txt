PROJECT(osgpangofonts)

# osgPango gets the proper debug postfix automatically since it's part of the
# current project, but we need to add the postfix for the other libs.
LINK_LIBRARIES(${OSGPANGO_LIBRARY_NAME}
    ${OSG_LIBRARY} ${OSGUTIL_LIBRARY} ${OSGDB_LIBRARY} ${OSGGA_LIBRARY} ${OSGTEXT_LIBRARY} ${OSGVIEWER_LIBRARY} ${OPENTHREADS_LIBRARY}
)

ADD_EXECUTABLE(osgpangofonts osgpangofonts.cpp)

INSTALL(TARGETS osgpangofonts DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangofonts PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
SET_TARGET_PROPERTIES(osgpangofonts PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OSGPANGO_ROOT}/bin)
