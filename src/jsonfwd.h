#pragma once
#ifndef DVFORMSCPP_JSONVALUEFWD_H
#define DVFORMSCPP_JSONVALUEFWD_H

#include <stddef.h>           // for size_t
#include <boost/variant.hpp>  // for variant
#include <cstdint>            // for int64_t
#include <iosfwd>             // for ostream, nullptr_t
#include <memory>             // for shared_ptr
#include <string>             // for string
#include <unordered_map>      // for unordered_map
#include <vector>             // for vector
#include <sstream>
#include <type_traits>        // for is_convertible, remove_const, remove_reference

#ifdef __has_attribute
  #if __has_attribute( pure )
    #define PURE __attribute__((pure))
  #else
    #define PURE
  #endif
#else
  #define PURE
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
    template<unsigned N> struct PriorityTag : PriorityTag<N - 1> {};
    template<> struct PriorityTag<0> {};

    struct JSONTypes {
      typedef std::nullptr_t nullType;
      typedef int64_t intType;
      typedef bool boolType;
      typedef double doubleType;
      typedef std::string stringType;
      typedef std::unordered_map<stringType, JSONPtr> objectType;
      typedef std::vector<JSONPtr> arrayType;
      typedef stringType keyType;
      typedef boost::variant<nullType, intType, boolType, doubleType, stringType, objectType, arrayType> valueType;
      typedef size_t indexType;
    };

    std::ostream &operator<<( std::ostream &os, const JSON &json );
    std::ostream &operator<<( std::ostream &os, const JSONDiffListenerImpl &listener );

    template<typename T>
    inline bool operator==( T &&v, const JSON &j ) {
      return j == v;
    }

    template<typename T>
    inline bool operator!=( T &&v, const JSON &j ) {
      return j != v;
    }

    namespace detail {
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
          static const bool value = has_type<typename std::remove_reference<typename std::remove_const<X>::type>::type, Y>::value;
        };
      template<typename X, typename Y> struct variant_has_type<const X, Y> : public variant_has_type<X, Y> {};
      template<typename X, typename Y> struct variant_has_type<X &, Y> : public variant_has_type<X, Y> {};

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
          static const bool value = is_convertible<X, Y>::value;
        };
      template<typename X, typename Y> struct variant_is_convertible<const X, Y> : public variant_is_convertible<X, Y> {};

      static_assert( variant_is_convertible<const char[], JSONTypes::valueType>::value, "Should be convertible" );

      static_assert( variant_has_type<std::string, JSONTypes::valueType>::value, "should have" );
      static_assert( variant_has_type<const std::string, JSONTypes::valueType>::value, "should have" );
      static_assert( variant_has_type<std::string &, JSONTypes::valueType>::value, "should have" );
      static_assert( variant_has_type<const std::string &, JSONTypes::valueType>::value, "should have" );
      static_assert( !variant_has_type<float, JSONTypes::valueType>::value, "shouldn't have" );
    }
  }
}

#endif //DVFORMSCPP_JSONVALUEFWD_H
