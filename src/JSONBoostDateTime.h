#pragma once
#ifndef DVMON_JSONBOOSTDATETIME_H
#define DVMON_JSONBOOSTDATETIME_H

#include "jsonfwd.h"
#include "JSONValue.h"
#include "JSONBoostOptional.h"
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/special_defs.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <string>

namespace dv {
  namespace json {
    inline void to_json( JSON &j, const boost::posix_time::time_duration &d, const JSONPath & ) {
      j = boost::convert<std::string>( d, boost::cnv::lexical_cast() );
    }

    inline void from_json( const JSON &j, boost::posix_time::time_duration &d, const JSONPath & ) {
      auto value = boost::convert<boost::posix_time::time_duration>( j.as<std::string>(), boost::cnv::lexical_cast() );
      if ( value ) {
        d = value.get();
      } else {
        d = boost::posix_time::not_a_date_time;
      }
    }
  }
}

#endif //DVMON_JSONBOOSTDATETIME_H
