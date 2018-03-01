#pragma once
#ifndef DVJSON_JSONSERIALISER_H
#define DVJSON_JSONSERIALISER_H

#include "jsonfwd.h"                // for JSONErrorCollectorPtr, defaultErrorCollector, emptyPath, JSONTypes, JSONTypes::valueType, JSONTypes::stringType
#include "JSONErrorCollector.h"     // for JSONErrorCollector
#include "JSONContext.h"            // for JSONContext
#include "JSONPath.h"
#include <stdlib.h>                 // for abs
#include <algorithm>                // for forward
#include <boost/core/demangle.hpp>  // for demangle
#include <boost/variant.hpp>        // for static_visitor
#include <cstddef>                  // for nullptr_t
#include <limits>                   // for numeric_limits
#include <memory>                   // for shared_ptr, weak_ptr, __shared_ptr_access
#include <type_traits>              // for declval, enable_if, is_floating_point, remove_reference, remove_const, is_convertible, is_integral, is_same
#include <typeinfo>                 // for type_info

//#define DISABLE_JSON_MISSING_FUNC

namespace dv {
  namespace json {
    class JSON;
    class JSONPath;
    namespace detail {
      struct to_json_function {
       private:
        template<typename JsonType, typename T, typename std::enable_if<variant_has_type<T, JSONTypes::valueType>::value, int>::type = 0>
        void call( JsonType &j, T &&val, const JSONPath &/*path*/, PriorityTag<10> ) const {
          j.value = val;
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, T &&value, const JSONPath &/*path*/, PriorityTag<9> ) const
        -> decltype( j.value = JSONTypes::stringType( value ), void() ) {
          j.value = JSONTypes::stringType( value );
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, std::shared_ptr<T> &&val, const JSONPath &path, PriorityTag<6> ) const
        noexcept( noexcept( to_json( j, std::forward<std::shared_ptr<T >>( val ), path ) ) )
        -> decltype( to_json( j, std::forward<std::shared_ptr<T >>( val ), path ), void() ) {
          static_assert( has_to_json<T>::value, "has_to_json returned false" );
          return to_json( j, std::forward<std::shared_ptr<T >>( val ), path );
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, std::weak_ptr<T> &&val, const JSONPath &path, PriorityTag<5> ) const
        noexcept( noexcept( to_json( j, std::forward<std::weak_ptr<T >>( val ), path ) ) )
        -> decltype( to_json( j, std::forward<std::weak_ptr<T >>( val ), path ), void() ) {
          static_assert( has_to_json<T>::value, "has_to_json returned false" );
          return to_json( j, std::forward<std::weak_ptr<T >>( val ), path );
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, T &&val, const JSONPath &path, PriorityTag<4> ) const
        noexcept( noexcept( to_json( j, std::forward<T>( val ), path ) ) )
        -> decltype( to_json( j, std::forward<T>( val ), path ), void() ) {
          static_assert( has_to_json<T>::value, "has_to_json returned false" );
          return to_json( j, std::forward<T>( val ), path );
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, T &&val, const JSONPath &path, PriorityTag<4> ) const
        noexcept( noexcept( std::declval<T>().toJson( j, std::declval<JSONPath &>() ) ) )
        -> decltype( std::declval<T>().toJson( j, std::declval<JSONPath &>() ), void() ) {
          return val.toJson( j, path );
        }

        template<typename JsonType, typename T>
        auto call( JsonType &j, T &&val, const JSONPath &/*path*/, PriorityTag<3> ) const
        noexcept( noexcept( std::declval<T>().toJson( j ) ) )
        -> decltype( std::declval<T>().toJson( j ), void() ) {
          return val.toJson( j );
        }

        template<typename JsonType, typename T, enable_if_t<is_streamable_object<T>::value> = 0>
        void call( JsonType &j, T &&val, const JSONPath &/*path*/, PriorityTag<2> ) const {
          std::ostringstream stream;
          stream << val;
          j = stream.str();
        }

#ifndef DISABLE_JSON_MISSING_FUNC

        template<typename JsonType, typename T>
        void call( JsonType &, T &&, const JSONPath &/*path*/, PriorityTag<0> ) const noexcept {
          static_assert( sizeof( JsonType ) == 0, "could not find to_json() method in T's namespace" );
        }

#endif

       public:
        template<typename JsonType, typename T>
        auto operator()( JsonType &j, T &&val, const JSONPath &path ) const
        noexcept( noexcept( std::declval<to_json_function>().call( j, std::forward<T>( val ), path, PriorityTag<10> {} ) ) )
        -> decltype( call( j, std::forward<T>( val ), path, PriorityTag<10> {} ) ) {
          return call( j, std::forward<T>( val ), path, PriorityTag<10> {} );
        }
      };

      inline std::ostream &operator<<( std::ostream &os, std::nullptr_t ) {
        os << "null";
        return os;
      }

      struct from_json_function {
       private:
        template<typename O> struct visitor : public boost::static_visitor<void> {
          visitor( O o, const JSON &json, const JSONPath &nPath )
            : oo( o ), j( json ), path( nPath ) {}

          template<typename JsonType, typename Current, typename Other=O>
          auto call( const JsonType &, Current &, Other &&other, PriorityTag<6> ) const
          -> decltype( from_json( std::declval<const JSON &>(), other, std::declval<JSONPath &>() ), void() ) {
            from_json( j, other, path );
          }

          template<typename JsonType, typename Current, typename Other=O>
          auto call( const JsonType &, Current &, Other &&other, PriorityTag<5> ) const
          -> decltype( std::declval<Other>().fromJson( std::declval<const JSON &>(), std::declval<const JSONPath &>() ), void() ) {
            other.fromJson( j, path );
          }

          template<typename JsonType, typename Current, typename Other=O,
            detail::enable_if_t<std::is_convertible<detail::uncvref_t<Other>, Current>::value> = 0>
          auto call( const JsonType &, Current &val, Other &&other, PriorityTag<4> ) const
          -> decltype( other = val, void() ) {
            other = val;
          }

          template<typename JsonType, typename Current, typename Other=O>
          auto call( const JsonType &, Current &, Other &&other, PriorityTag<3> ) const
          -> decltype( std::declval<Other>().fromJson( std::declval<const JSON &>() ), void() ) {
            other.fromJson( j );
          }

          template<typename JsonType, typename Current, typename Other=O, enable_if_t<is_streamable_object<Other>::value and
                                                                                      ( std::is_fundamental<Current>::value or
                                                                                        is_streamable_object<Current>::value )> = 0>
          void call( const JsonType &, Current &val, Other &&other, PriorityTag<2> ) const {
            std::stringstream stream;
            stream << val;
            stream >> other;
          }

          template<typename JsonType, typename Current, typename Other=O,
            typename std::enable_if<variant_is_convertible<Other, JSONTypes::valueType>::value, int>::type = 0>
          void call( JsonType &jo, Current &&, Other &&, PriorityTag<1> ) const {
            JSONContext::current()->get<JSONErrorCollector>()->
              error( path, "Can't convert " + jo.type() + " to " + boost::core::demangle( typeid( Other ).name() ) );
          }

#ifndef DISABLE_JSON_MISSING_FUNC

          template<typename JsonType, typename Current, typename Other=O>
          void call( const JsonType &, Current &, Other &&, PriorityTag<0> ) const {
            static_assert( sizeof( JsonType ) == 0, "could not find from_json() method in T's namespace" );
          }

#endif
          O oo;
          const JSON &j;
          const JSONPath &path;

          template<typename X>
          void operator()( X &&val ) const noexcept {
            return call( j, std::forward<X>( val ), oo, PriorityTag<10> {} );
          }
        };
       public:
        template<typename JsonType, typename T>
        T &operator()( JsonType &j, T &&val, const JSONPath &path ) const {
          visitor<T> v( std::forward<T>( val ), j, path );
          j.value.apply_visitor( v );
          return val;
        }
      };

      template<typename A, typename B> struct are_comparable {
        typedef typename std::remove_reference<typename std::remove_const<A>::type>::type X;
        typedef typename std::remove_reference<typename std::remove_const<B>::type>::type Y;
        static const bool floatValue =
          ( std::is_floating_point<X>::value && std::is_integral<Y>::value ) ||
          ( std::is_floating_point<Y>::value && std::is_integral<X>::value ) ||
          ( std::is_floating_point<X>::value && std::is_floating_point<Y>::value );
        static const bool value = std::is_convertible<X, Y>::value && !floatValue;
      };

      struct json_compare_function {
       private:
        template<typename O> struct visitor : public boost::static_visitor<bool> {
          visitor( O o, const JSON &json, const JSONPath &nPath )
            : oo( o ), j( json ), path( nPath ) {}

          template<typename JsonType, typename Other=O>
          bool call( JsonType &, std::nullptr_t &, Other &&, PriorityTag<10> ) const noexcept {
            return std::is_same<typename std::remove_const<typename std::remove_reference<Other>::type>::type, std::nullptr_t>::value;
          }

          template<typename JsonType, typename Current, typename Other=O, typename std::enable_if<are_comparable<Current, Other>::floatValue, int>::type = 0>
          bool call( JsonType &, Current &&value, Other &&other, PriorityTag<8> ) const {
            return std::abs( other - value ) <= std::numeric_limits<JSONTypes::doubleType>::epsilon();
          }

          template<typename JsonType, typename Current, typename Other=O, typename std::enable_if<are_comparable<Other, Current>::value, int>::type = 0>
          auto call( JsonType &, Current &&value, Other &&other, PriorityTag<7> ) const -> decltype( value == other ) {
            return value == other;
          }

          template<typename JsonType, typename Current, typename Other=O>
          auto call( const JsonType &, Current &, Other &&other, PriorityTag<6> ) const
          noexcept( noexcept( json_compare( std::declval<JsonType>(), std::declval<Other>(), std::declval<JSONPath &>() ) ) ) ->
          decltype( json_compare( std::declval<JsonType>(), std::declval<Other>(), std::declval<JSONPath &>() ) ) {
            return json_compare( j, other, path );
          }

          template<typename JsonType, typename Current, typename Other=O,
            typename std::enable_if<variant_has_type<Other, JSONTypes::valueType>::value, int>::type = 0>
          bool call( JsonType &, Current &&, Other &&, PriorityTag<5> ) const {
            return false;
          }

#ifndef DISABLE_JSON_MISSING_FUNC

          template<typename JsonType, typename Current, typename Other=O>
          bool call( const JsonType &, Current &, Other &&, PriorityTag<0> ) const noexcept {
            static_assert( sizeof( JsonType ) == 0, "could not find json_compare() method in T's namespace" );
            return false;
          }

#endif
          O oo;
          const JSON &j;
          const JSONPath &path;

          template<typename X>
          bool operator()( X &&val ) const {
            return call( j, std::forward<X>( val ), oo, PriorityTag<10> {} );
          }
        };

       public:
        template<typename JsonType, typename T>
        bool operator()( JsonType &j, T &&val, const JSONPath &path ) const {
          visitor<T> v( std::forward<T>( val ), j, path );
          return j.value.apply_visitor( v );
        }
      };

      struct json_construct_function {
       private:
        template<typename JsonType, typename T>
        auto call( JsonType *j, T *val, PriorityTag<1> ) const noexcept( noexcept( json_construct( val, j ) ) )
        -> decltype( json_construct( val, j ) ) {
          return json_construct( val, j );
        }

        template<typename JsonType, typename T>
        T call( const JsonType *, T *, PriorityTag<0> ) const {
          return T();
        }

       public:
        template<typename JsonType, typename T>
        auto
        operator()( JsonType *j, T *val ) const noexcept( noexcept( std::declval<json_construct_function>().call( j, val, PriorityTag<1> {} ) ) )
        -> decltype( std::declval<json_construct_function>().call( j, val, PriorityTag<1> {} ) ) {
          return call( j, val, PriorityTag<1> {} );
        }
      };

      template<typename T>
        struct static_const {
          static constexpr T value{};
        };

      template<typename T>
        constexpr T static_const<T>::value;
    }

    namespace {
      constexpr const auto &to_json = detail::static_const<detail::to_json_function>::value;
      constexpr const auto &from_json = detail::static_const<detail::from_json_function>::value;
      constexpr const auto &json_compare = detail::static_const<detail::json_compare_function>::value;
      constexpr const auto &json_construct = detail::static_const<detail::json_construct_function>::value;
    }

    template<typename T = void>
      struct JSONSerialiser {
        template<typename JsonType, typename ValueType>
        static auto
        from_json( JsonType &&j, ValueType &val, const JSONErrorCollectorPtr &collector, const JSONPath &path )
        noexcept( noexcept( ::dv::json::from_json( std::forward<JsonType>( j ), val, path ) ) )
        -> decltype( ::dv::json::from_json( std::forward<JsonType>( j ), val, path ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::from_json( std::forward<JsonType>( j ), val, path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        from_json( JsonType &&j, ValueType &val, const JSONErrorCollectorPtr &collector )
        noexcept( noexcept( ::dv::json::from_json( std::forward<JsonType>( j ), val, std::declval<JSONPath &>() ) ) )
        -> decltype( ::dv::json::from_json( std::forward<JsonType>( j ), val, std::declval<JSONPath &>() ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::from_json( std::forward<JsonType>( j ), val, path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        from_json( JsonType &&j, ValueType &val, const JSONPath &path )
        noexcept( noexcept( ::dv::json::from_json( std::forward<JsonType>( j ), val, path ) ) )
        -> decltype( ::dv::json::from_json( std::forward<JsonType>( j ), val, path ) ) {
          JSONContext context;
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::from_json( std::forward<JsonType>( j ), val, path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        from_json( JsonType &&j, ValueType &val )
        noexcept( noexcept( ::dv::json::from_json( std::forward<JsonType>( j ), val, std::declval<const JSONPath &>() ) ) )
        -> decltype( ::dv::json::from_json( std::forward<JsonType>( j ), val, std::declval<const JSONPath &>() ) ) {
          JSONContext context;
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::from_json( std::forward<JsonType>( j ), val, *path );
        }

        // ======================================

        template<typename JsonType, typename ValueType>
        static auto
        to_json( JsonType &j, ValueType &&val, const JSONErrorCollectorPtr &collector, const JSONPath &path )
        noexcept( noexcept( ::dv::json::to_json( j, std::forward<ValueType>( val ), path ) ) )
        -> decltype( ::dv::json::to_json( j, std::forward<ValueType>( val ), path ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::to_json( j, std::forward<ValueType>( val ), path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        to_json( JsonType &j, ValueType &&val, const JSONErrorCollectorPtr &collector )
        noexcept( noexcept( ::dv::json::to_json( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) )
        -> decltype( ::dv::json::to_json( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::to_json( j, std::forward<ValueType>( val ), *path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        to_json( JsonType &j, ValueType &&val, const JSONPath &path )
        noexcept( noexcept( ::dv::json::to_json( j, std::forward<ValueType>( val ), path ) ) )
        -> decltype( ::dv::json::to_json( j, std::forward<ValueType>( val ), path ) ) {
          JSONContext context;
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::to_json( j, std::forward<ValueType>( val ), path );
        }

        template<typename JsonType, typename ValueType>
        static auto
        to_json( JsonType &j, ValueType &&val )
        noexcept( noexcept( ::dv::json::to_json( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) )
        -> decltype( ::dv::json::to_json( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) {
          JSONContext context;
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::to_json( j, std::forward<ValueType>( val ), *path );
        }

        // ======================================

        template<typename JsonType, typename ValueType>
        static bool compare( JsonType &j, ValueType &&val, const JSONErrorCollectorPtr &collector, const JSONPath &path )
        noexcept( noexcept( ::dv::json::json_compare( j, std::forward<ValueType>( val ), path ) ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::json_compare( j, std::forward<ValueType>( val ), path );
        }

        template<typename JsonType, typename ValueType>
        static bool compare( JsonType &j, ValueType &&val, const JSONErrorCollectorPtr &collector )
        noexcept( noexcept( ::dv::json::json_compare( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) ) {
          JSONContext context;
          context.attach( collector );
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::json_compare( j, std::forward<ValueType>( val ), *path );
        }

        template<typename JsonType, typename ValueType>
        static bool compare( JsonType &j, ValueType &&val, const JSONPath &path )
        noexcept( noexcept( ::dv::json::json_compare( j, std::forward<ValueType>( val ), path ) ) ) {
          JSONContext context;
          auto cr = context.enter();
          context.attach( path );
          return ::dv::json::json_compare( j, std::forward<ValueType>( val ), path );
        }

        template<typename JsonType, typename ValueType>
        static bool compare( JsonType &j, ValueType &&val )
        noexcept( noexcept( ::dv::json::json_compare( j, std::forward<ValueType>( val ), std::declval<const JSONPath &>() ) ) ) {
          JSONContext context;
          auto cr = context.enter();
          auto path = context.get<JSONPath>();
          return ::dv::json::json_compare( j, std::forward<ValueType>( val ), *path );
        }
      };

    template<typename T = void>
      struct JSONConstructor {
        typedef decltype( ::dv::json::json_construct( std::declval<JSON *>(), std::declval<T *>() ) ) constructType;

        static constructType construct( T *val ) {
          return ::dv::json::json_construct( static_cast<JSON *>(nullptr), val );
        }
      };

    template<typename T> struct JSONConstructor<T &> : public JSONConstructor<T> {};
    template<typename T> struct JSONConstructor<const T> : public JSONConstructor<T> {};
  }
}
#endif //DVJSON_JSONSERIALISER_H
