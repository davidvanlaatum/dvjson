#pragma once
#ifndef DVMON_JSONBOOSTPOINTERS_H
#define DVMON_JSONBOOSTPOINTERS_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "jsonfwd.h"
#include "JSONSerialiser.h"
#include "JSONValue.h"

namespace dv {
  namespace json {

    template<typename T>
    void to_json( JSON &json, const boost::shared_ptr<T> &ptr, const JSONPath &path ) {
      if ( ptr ) {
        JSONSerialiser<T>::to_json( json, *ptr, path );
      } else {
        json = nullptr;
      }
    }

    template<typename T, typename std::enable_if<std::is_base_of<boost::enable_shared_from_this<T>, T>::value, int>::type = 0>
    boost::shared_ptr<T> json_construct( T *, JSON * ) {
      return boost::make_shared<T>();
    }
  }
}

#endif //DVMON_JSONBOOSTPOINTERS_H
