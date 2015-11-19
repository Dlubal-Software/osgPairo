PROJECT(osgpangoguiviewer)

# osgPango gets the proper debug postfix automatically since it's part of the
# current project, but we need to add the postfix for the other libs.
LINK_LIBRARIES(osgPango
    ${OSG_LIBRARY} ${OSGUTIL_LIBRARY} ${OSGDB_LIBRARY} ${OSGGA_LIBRARY} ${OSGTEXT_LIBRARY} ${OSGVIEWER_LIBRARY} ${OPENTHREADS_LIBRARY}
)

ADD_EXECUTABLE(osgpangoguiviewer osgpangoguiviewer.cpp)

INSTALL(TARGETS osgpangoguiviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangoguiviewer PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
