add_library ( dvjson SHARED
              json.h
              JSONParser.cpp JSONParser.h
              JSONPath.cpp JSONPath.h
              JSONValue.cpp JSONValue.h
              jsonfwd.h
              JSONSerialiser.h
              JSONDiffListener.cpp JSONDiffListener.h
              JSONConverters.h
              JSONException.cpp JSONException.h
              JSONErrorCollector.cpp JSONErrorCollector.h
              JSONContext.h JSONContext.cpp
              UnorderedIndexedMap.h
              JSONBoostSerialise.h
              JSONBoostOptional.h
              JSONBoostPointers.h
              JSONBoostDateTime.h )
target_link_libraries ( dvjson PUBLIC Boost::boost )
set_target_properties ( dvjson PROPERTIES VERSION ${PROJECT_VERSION} )
if ( DV_LIB_OUTPUT_DIR )
  set_target_properties ( dvjson PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${DV_LIB_OUTPUT_DIR} )
else ()
  message ( FATAL_ERROR "bla" )
endif ()
get_target_property ( HEADERS dvjson SOURCES )
list ( FILTER HEADERS INCLUDE REGEX ".h$" )
set_target_properties ( dvjson PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install ( TARGETS dvjson EXPORT dvjson
          LIBRARY COMPONENT lib DESTINATION ${CMAKE_INSTALL_LIBDIR} NAMELINK_SKIP
          PUBLIC_HEADER COMPONENT Devel DESTINATION include/dvjson/ )
install ( TARGETS dvjson EXPORT dvjson LIBRARY NAMELINK_ONLY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Devel )
target_include_directories ( dvjson PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/dvjson/> )
if ( NOT SUBBUILD_DVJSON )
  iwyu_target ( dvjson )
  clang_tidy_target ( dvjson )
endif ()
