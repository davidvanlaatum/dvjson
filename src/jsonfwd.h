#pragma once
#ifndef DVJSON_JSONVALUEFWD_H
#define DVJSON_JSONVALUEFWD_H

#include <boost/variant.hpp>  // for variant
#include <boost/type_traits.hpp>
#include <cstddef>            // for size_t
#include <cstdint>            // for int64_t
#include <iosfwd>             // for ostream, nullptr_t
#include <memory>             // for shared_ptr
#include <string>             // for string
#include <unordered_map>      // for unordered_map
#include <vector>             // for vector
#include <sstream>
#include <type_traits>        // for is_convertible, remove_const, remove_reference
#include "UnorderedIndexedMap.h"

#ifdef __has_attribute
  #if __has_attribute( pure )
    #define JSON_PURE __attribute__((pure))
  #else
    #define JSON_PURE
  #endif
  #if __has_attribute( unused )
    #define JSON_UNUSED __attribute__((unused))
  #else
    #define JSON_UNUSED
  #endif
#else
  #define JSON_PURE
  #define JSON_UNUSED
#endif

namespace dv {
  namespace json {
    enum class Type {
      OBJECT,
      ARRAY,
      BOOL,
      INT,
      DOUBLE,
      STRING,
      NULLVALUE
    };

    std::ostream &operator<<( std::ostream &os, Type type );

    inline std::string operator+( const std::string &s, Type type ) {
      std::ostringstream str;
      str << s << type;
      return str.str();
    }

    class JSON;
    typedef std::shared_ptr<JSON> JSONPtr;
    class JSONPath;// IWYU pragma: keep
    class JSONDiffListener;// IWYU pragma: keep
    class JSONDiffListenerImpl;
    class JSONErrorCollector;
    typedef std::shared_ptr<JSONErrorCollector> JSONErrorCollectorPtr;
    class JSONErrorCollectorThrow; // IWYU pragma: keep
    class JSONErrorCollectorImpl; // IWYU pragma: keep
    class JSONException;
    class JSONParseException;
    template<unsigned N> struct PriorityTag : PriorityTag<N - 1> {};
    template<> struct PriorityTag<0> {};

    struct JSONTypes {
      typedef std::nullptr_t nullType;
      typedef int64_t intType;
      typedef bool boolType;
      typedef double doubleType;
      typedef std::string stringType;
      typedef UnorderedIndexedMap<std::string, JSONPtr> objectType;
      typedef std::vector<JSONPtr> arrayType;
      typedef stringType keyType;
      typedef boost::variant<nullType, intType, boolType, doubleType, stringType, objectType, arrayType> valueType;
      typedef size_t indexType;
    };

    namespace detail {
      template<bool B> using enable_if_t = typename std::enable_if<B, int>::type;
      template<typename T> using uncvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

      template<typename X, typename Y>
        struct variant_has_type {
         private:
          template<typename T, typename V>
            struct has_type;

          template<typename T, typename... Ts>
            struct has_type<T, boost::variant<T, Ts...> > {
              static const bool value = true;
            };

          template<typename T, typename U, typename... Ts>
            struct has_type<T, boost::variant<U, Ts...> > : has_type<T, boost::variant<Ts..., void> > {};

          template<typename T, typename... Ts>
            struct has_type<T, boost::variant<void, Ts...> > {
              static const bool value = false;
            };
         public:
          static const bool value = has_type<uncvref_t<X>, uncvref_t<Y>>::value;
        };

      template<typename X, typename Y>
        struct variant_is_convertible {
         private:
          template<typename T, typename V>
            struct is_convertible;

          template<typename T, typename V, typename... Ts>
            struct is_convertible<T, boost::variant<V, Ts...>> {
              static const bool value = std::is_convertible<T, V>::value || is_convertible<T, boost::variant<Ts..., void>>::value;
            };

          template<typename T, typename... Ts>
            struct is_convertible<T, boost::variant<void, Ts...>> {
              static const bool value = false;
            };
         public:
          static const bool value = not std::is_same<uncvref_t<X>, Y>::value and is_convertible<uncvref_t<X>, Y>::value;
        };

      template<typename Y, typename Z=uncvref_t<Y>> struct is_streamable_object_sub {
       private:
        template<typename T> struct has_left {
          template<typename X>
          static auto check( X * ) -> decltype( std::declval<std::ostream &>() << std::declval<X &>(), std::true_type() );
          template<typename> static constexpr std::false_type check( ... );
          typedef decltype( check<T>( nullptr ) ) type;
          static const bool value = type::value;
        };
        template<typename T> struct has_right {
          template<typename X>
          static auto check( X * ) -> decltype( std::declval<std::istream>() >> std::declval<X>(), std::true_type() );
          template<typename> static constexpr std::false_type check( ... );
          typedef decltype( check<T>( nullptr ) ) type;
          static const bool value = type::value;
        };
       public:
        static const bool value = not std::is_fundamental<Z>::value and
                                  std::is_object<Z>::value and
                                  has_left<Z>::value and
                                  has_right<Z>::value;
      };
      template<> struct is_streamable_object_sub<std::nullptr_t> { static const bool value = false; };

      template<typename T, typename X=uncvref_t<T>>
        struct is_streamable_object : public std::conditional<is_streamable_object_sub<X>::value, std::true_type, std::false_type>::type {};

      template<typename T>
        struct has_to_json {
         private:
          template<typename X, typename JSONType=JSON>
          static auto check( X * )
          -> decltype( to_json( std::declval<JSONType &>(), std::forward<X>( std::declval<X>() ), std::declval<JSONPath &>() ), std::true_type() );
          template<typename> static constexpr std::false_type check( ... );
          typedef decltype( check<uncvref_t<T>>( 0 ) ) type;
         public:
          static const bool value = type::value;
        };

      template<typename T>
        struct supports_implicit_to_json {
          static const bool value =
            has_to_json<T>::value or
            variant_has_type<T, JSONTypes::valueType>::value or
            variant_is_convertible<T, JSONTypes::valueType>::value or
            is_streamable_object<T>::value;
        };

      template<typename T, typename X=uncvref_t<T>>
        struct supports_implicit_json_compare {
          static const bool value =
            not std::is_same<X, JSON>::value and
            not std::is_same<X, JSONPtr>::value;
        };

      void writeJSON( std::ostream &os, const JSON &json );
      void readJSON( std::istream &is, JSON &json );
    }

    template<typename T, typename JSONType=JSON, detail::enable_if_t<detail::supports_implicit_json_compare<T>::value && std::is_same<T, JSONType>::value> = 0>
    inline bool operator==( T &&v, const JSONType &j ) {
      return j == v;
    }

    template<typename T, typename JSONType=JSON, detail::enable_if_t<detail::supports_implicit_json_compare<T>::value && std::is_same<T, JSONType>::value> = 0>
    inline bool operator!=( T &&v, const JSONType &j ) {
      return j != v;
    }

    template<typename T, typename X=detail::uncvref_t<T>, detail::enable_if_t<std::is_same<X, JSON>::value> = 0>
    inline std::ostream &operator<<( std::ostream &os, const T &json ) {
      detail::writeJSON( os, json );
      return os;
    }
    std::ostream &operator<<( std::ostream &os, const JSONDiffListenerImpl &listener );

    template<typename T, typename X=detail::uncvref_t<T>, detail::enable_if_t<std::is_same<X, JSON>::value> = 0>
    inline std::istream &operator>>( std::istream &is, T &json ) {
      detail::readJSON( is, json );
      return is;
    }

    inline void PrintTo( const JSON &j, ::std::ostream *os ) {
      detail::writeJSON( *os, j );
    }
  }
}

#endif //DVJSON_JSONVALUEFWD_H
