#pragma once
#ifndef DVMON_JSONBOOSTOPTIONAL_H
#define DVMON_JSONBOOSTOPTIONAL_H

#include "jsonfwd.h"
#include "JSONSerialiser.h"
#include "JSONValue.h"
#include <boost/optional/optional.hpp>

namespace dv {
  namespace json {
    template<typename T> inline void to_json( JSON &j, const boost::optional<T> &value, const JSONPath &path ) {
      if ( value.is_initialized() ) {
        JSONSerialiser<T>::to_json( j, value.value(), path );
      } else {
        j = nullptr;
      }
    }

    template<> inline void to_json( JSON &j, const boost::optional<JSON> &value, const JSONPath &/*path*/ ) {
      if ( value.is_initialized() ) {
        j = value.get();
      } else {
        j = nullptr;
      }
    }

    template<typename T> inline void from_json( const JSON &j, boost::optional<T> &value, const JSONPath &path ) {
      if ( j == nullptr ) {
        value.reset();
      } else {
        T tmp;
        JSONSerialiser<T>::from_json( j, tmp, path );
        value = tmp;
      }
    }

    template<> inline void from_json( const JSON &j, boost::optional<JSON> &value, const JSONPath &/*path*/) {
      if ( j == nullptr ) {
        value.reset();
      } else {
        value = j;
      }
    }

    template<typename T> inline JSON &json_as( T &, JSON &j, boost::optional<JSON> * ) {
      return j;
    }

    template<typename T> inline const JSON &json_as( T &, const JSON &j, boost::optional<JSON> * ) {
      return j;
    }

    namespace detail {
      template<> struct is_streamable_object_sub<boost::optional<JSON>> { static const bool value = false; };
    }
  }
}

#endif //DVMON_JSONBOOSTOPTIONAL_H
