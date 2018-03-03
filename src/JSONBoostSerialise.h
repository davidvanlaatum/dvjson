#pragma once
#ifndef DVMON_JSONBOOSTSERIALISE_H
#define DVMON_JSONBOOSTSERIALISE_H

#include "jsonfwd.h"
#include <JSONParser.h>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>

namespace dv {
  namespace json {

    template<class A, typename JSONType=JSON, typename std::enable_if<std::is_same<JSONType, JSON>::value
                                                                      and std::is_base_of<boost::archive::detail::interface_oarchive<A>, A>::value,
      int>::type = 0>
    inline A &operator<<( A &a, const JSONType &j ) {
      a & to_string( j );
      return a;
    }

    template<class A, typename JSONType=JSON, typename std::enable_if<std::is_same<JSONType, JSON>::value
                                                                      and std::is_base_of<boost::archive::detail::interface_iarchive<A>, A>::value,
      int>::type = 0>
    inline A &operator>>( A &a, JSONType &j ) {
      std::string value;
      a & value;
      JSONParser parser;
      parser.parseInto( j, value );
      return a;
    }
  }
}

#endif //DVMON_JSONBOOSTSERIALISE_H
