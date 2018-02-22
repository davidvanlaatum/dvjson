#pragma once
#ifndef DVMON_JSONBOOSTOPTIONAL_H
#define DVMON_JSONBOOSTOPTIONAL_H

#include "jsonfwd.h"
#include "JSONSerialiser.h"
#include <boost/optional/optional.hpp>

namespace dv {
  namespace json {
    template<typename T> void to_json( JSON &j, const boost::optional<T> &value, const JSONPath &path ) {
      if ( value.is_initialized() ) {
        JSONSerialiser<T>::to_json( j, value.value(), path );
      } else {
        j = nullptr;
      }
    }

    template<typename T> inline JSON &json_as( T &, JSON &j, boost::optional<JSON> * ) {
      return j;
    }

    template<typename T> inline const JSON &json_as( T &, const JSON &j, boost::optional<JSON> * ) {
      return j;
    }
  }
}

#endif //DVMON_JSONBOOSTOPTIONAL_H
