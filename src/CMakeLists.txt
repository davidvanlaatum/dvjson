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
        JSONContext.h JSONContext.cpp )
target_link_libraries ( dvjson PUBLIC Boost::boost )
set_target_properties ( dvjson PROPERTIES VERSION ${PROJECT_VERSION} )
get_target_property ( HEADERS dvjson SOURCES )
list ( FILTER HEADERS INCLUDE REGEX ".h$" )
set_target_properties ( dvjson PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install ( TARGETS dvjson EXPORT dvjson DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION include/dvjson/ )
target_include_directories ( dvjson PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/dvjson/> )
iwyu_target ( dvjson )
clang_tidy_target ( dvjson )
