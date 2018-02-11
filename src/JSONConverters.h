#pragma once
#ifndef DVFORMSCPP_JSONCONVERTERS_H
#define DVFORMSCPP_JSONCONVERTERS_H

#include "JSONSerialiser.h"   // for variant_has_type, JSONSerialiser
#include "JSONValue.h"        // for JSON
#include "jsonfwd.h"          // for JSONTypes::intType, JSONTypes::stringType, JSONTypes::valueType, Type, JSONTypes::doubleType, Type::DOUBLE, Type::INT
#include "JSONPath.h"
#include <boost/variant.hpp>  // for static_visitor, variant
#include <map>                // for map
#include <memory>             // for shared_ptr, __shared_ptr_access, enable_shared_from_this, make_shared, weak_ptr
#include <type_traits>        // for enable_if, is_integral, is_base_of, is_floating_point
#include <unordered_map>      // for unordered_map
#include <vector>             // for vector
#include <assert.h>           // for assert
#include <stddef.h>           // for size_t

namespace dv {
  namespace json {
    template<typename T>
    inline void to_json( JSON &json, const std::map<JSON::stringType, T> &v, const JSONPath &path ) {
      json = nullptr;
      for ( const auto &it : v ) {
        JSONSerialiser<T>::to_json( json[it.first], it.second, path / it.first );
      }
    }

    template<typename T, typename std::enable_if<!std::is_same<T, JSON>::value, int>::type = 0>
    inline void to_json( JSON &json, const std::unordered_map<JSON::stringType, T> &v, const JSONPath &path ) {
      json = nullptr;
      for ( const auto &it : v ) {
        JSONSerialiser<T>::to_json( json[it.first], it.second, path / it.first );
      }
    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    inline void to_json( JSON &json, T v, const JSONPath &/*path*/ ) {
      json = static_cast<JSON::intType>(v);
    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    inline void from_json( const JSON &json, T &v, const JSONPath &/*path*/ ) {
      v = static_cast<T>(json.as<JSON::intType>());
    }

    template<typename T>
    inline void from_json( const JSON &json, const std::shared_ptr<T> &v, const JSONPath &path ) {
      assert( v );
      JSONSerialiser<T>::from_json( json, *v, path );
    }

    namespace detail {
      class to_json_visitor : public boost::static_visitor<> {
       public:
        explicit to_json_visitor( JSON &njson, const JSONPath &nPath )
          : json( &njson ), path( nPath ) {}

        template<class T> void operator()( const std::shared_ptr<T> &value ) {
          if ( value ) {
            ( *this )( *value );
          } else {
            *json = nullptr;
          }
        }

        template<class T> void operator()( const std::weak_ptr<T> &value ) {
          ( *this )( value.lock() );
        }

        template<typename T, typename std::enable_if<variant_has_type<T, JSON::valueType>::value, int>::type = 0>
        void operator()( T &value ) {
          *json = value;
        }

        template<class T, typename std::enable_if<!variant_has_type<T, JSON::valueType>::value, int>::type = 0>
        void operator()( T &value ) {
          JSONSerialiser<T>::to_json( *json, value, errorCollector );
        }

       protected:
        JSON *json;
        JSONErrorCollectorPtr errorCollector;
        const JSONPath &path;
      };
    }

    template<typename... Args>
    inline void to_json( JSON &json, const boost::variant<Args...> &value, const JSONPath &path ) {
      detail::to_json_visitor visitor( json, path );
      value.apply_visitor( visitor );
    }

    template<typename T>
    bool json_compare( const JSON &j, const std::vector<T> &value, const JSONPath &path ) {
      bool rt = true;

      if ( value.size() != j.size() ) {
        rt = false;
      } else {
        size_t i = 0;
        auto values = value.begin();
        for ( const auto &item : j.arrayIterator() ) {
          JSONPath p( path );
          if ( !JSONSerialiser<T>::compare( *item, *values, p[i] ) ) {
            rt = false;
            break;
          }
          ++values;
          i++;
        }
      }

      return rt;
    }

    template<typename T, typename std::enable_if<std::is_base_of<std::enable_shared_from_this<T>, T>::value, int>::type = 0>
    std::shared_ptr<T> json_construct( T *, JSON * ) {
      return std::make_shared<T>();
    }
  }
}

#endif //DVFORMSCPP_JSONCONVERTERS_H
