#pragma once
#ifndef DVJSON_JSONVALUE_H
#define DVJSON_JSONVALUE_H

#include <stddef.h>               // for size_t
#include <boost/variant.hpp>      // for static_visitor, type_info
#include <boost/variant/get.hpp>  // for get
#include <iosfwd>                 // for ostream, nullptr_t
#include <memory>                 // for enable_shared_from_this
#include <stdexcept>              // for runtime_error
#include <type_traits>            // for enable_if, is_same, is_convertible, is_fundamental
#include <unordered_map>          // for operator!=, unordered_map, _Node_const_iterator, _Node_iterator_base, unordered_map<>::const_iterator
#include <utility>                // for forward, pair
#include <vector>                 // for vector
#include "JSONSerialiser.h"       // for JSONConstructor, JSONSerialiser, PriorityTag
#include "jsonfwd.h"              // for JSONTypes::objectType, JSONTypes::arrayType, JSONPtr, Type, JSONTypes::keyType, JSONTypes::stringType, JSONTypes::v...

namespace dv {
  namespace json {
    class JSONDiffListener;
    class JSONPath;
    class JSON : public std::enable_shared_from_this<JSON>, public JSONTypes {
    public:
      JSON() = default;
      JSON( const JSON & ) = default;
      JSON( JSON && ) = default;

      template<typename T, detail::enable_if_t<detail::supports_implicit_to_json<T>::value> = 0>
      JSON( T &&v ) { *this = v; }

      JSON( std::initializer_list<JSON> v ) {
        *this = Type::ARRAY;
        for ( auto &item : v ) {
          emplaceBack( item );
        }
      }

      JSON( std::initializer_list<std::pair<keyType, JSON>> v ) {
        *this = Type::OBJECT;
        for ( auto &item : v ) {
          ( *this )[item.first] = item.second;
        }
      }

      ~JSON() = default;

      JSON &operator=( Type );
      JSON &operator=( const JSON &other );
      JSON &operator=( JSON &&other );
      template<typename T> inline JSON &operator=( const T &v );
      typedef JSON &reference;
      typedef const JSON &const_reference;
      reference operator[]( const keyType &key ) JSON_PURE;

      inline reference operator[]( const char *key ) JSON_PURE { return ( *this )[keyType( key )]; }

      reference operator[]( indexType index ) JSON_PURE;
      const_reference operator[]( const keyType &key ) const JSON_PURE;

      inline const_reference operator[]( const char *key ) const JSON_PURE { return ( *this )[keyType( key )]; }

      const_reference operator[]( indexType index ) const JSON_PURE;
      bool operator==( const JSON &other ) const;
      bool operator!=( const JSON &other ) const;
      bool operator==( Type ) const noexcept;
      bool operator!=( Type ) const noexcept;

      inline const std::type_info &innerType() const { return value.type(); }

      template<typename T, typename std::enable_if<not std::is_same<std::string::value_type, T>::value and
                                                   not std::is_pointer<T>::value, int>::type = 0>
      inline operator T() const noexcept { return as<T>(); }
      template<typename T> inline typename JSONConstructor<T>::constructType as() const;
      template<typename T> inline typename JSONConstructor<T>::constructType as( typename JSONConstructor<T>::constructType & ) const;
      template<typename T> inline bool is() const;
      Type type() const noexcept;
      inline JSONPtr sub( const keyType &key ) const noexcept;

      template<typename T> inline T sub( const keyType &key, T &&defaultValue ) const {
        const auto &val = sub( key );
        return val ? val->as<T>() : defaultValue;
      }

      bool compare( const JSON &other, JSONDiffListener &listener ) const;

      void dump( std::ostream &os, unsigned int indent = 0 ) const;

      inline JSONPtr emplaceBack( const JSONPtr &json );
      template<typename... T> inline JSONPtr emplaceBack( T &&... );
      inline size_t size() const noexcept;
      std::string toString() const;
      class ArrayIterator {
      public:
        explicit ArrayIterator( const arrayType *nArray ) : array( nArray ) {}

        typedef arrayType::iterator iterator;
        typedef arrayType::const_iterator const_iterator;

        inline const_iterator begin() const { return array->begin(); }

        inline const_iterator end() const { return array->end(); }

      private:
        const arrayType *array;
      };
      inline const ArrayIterator arrayIterator() const noexcept;

      class ObjectIterator {
      public:
        explicit ObjectIterator( const objectType *nObject ) : object( nObject ) {}

        typedef objectType::iterator iterator;
        typedef objectType::const_iterator const_iterator;

        inline const_iterator begin() const { return object->begin(); }

        inline const_iterator end() const { return object->end(); }

      private:
        const objectType *object;
      };
      inline const ObjectIterator objectIterator() const noexcept;
    protected:
      valueType value;
      void dump( std::ostream &os, unsigned int indent, unsigned int level ) const;
      void writeEscaped( std::ostream &os, const stringType &v ) const;
      bool compare( const JSON &other, JSONDiffListener &listener, const JSONPath &path ) const;
      friend struct detail::to_json_function;
      friend struct detail::from_json_function;
      friend struct detail::json_compare_function;
    };

    namespace detail {

      template<typename Ret, typename Wanted=Ret> struct get_json_value_visitor : public boost::static_visitor<Ret> {
        explicit get_json_value_visitor( const JSON *pj ) : j( pj ) {}

        get_json_value_visitor( const JSON *pj, typename JSONConstructor<Wanted>::constructType *pRet ) : j( pj ), ret( pRet ) {}

        template<typename Actual, typename LRet=Ret>
        auto call( Actual &value, const PriorityTag<4> & )
        -> decltype( json_as( std::declval<const Actual &>(), std::declval<JSON &>(), std::declval<LRet *>() ), LRet() ) {
          return json_as( value, *j, static_cast<LRet *>(nullptr) );
        }

        template<typename Actual, typename LRet=Ret>
        LRet call( Actual &value, const PriorityTag<3> &, typename std::enable_if<std::is_same<LRet, Actual>::value, int>::type = 0 ) {
          if ( ret ) {
            *ret = value;
          }
          return value;
        }

        template<typename Actual, typename LRet=Ret>
        LRet call( Actual &value, const PriorityTag<2> &, typename std::enable_if<std::is_convertible<LRet, Actual>::value, int>::type = 0 ) {
          if ( ret ) {
            *ret = static_cast<LRet>(value);
          }
          return static_cast<LRet>(value);
        }

        template<typename Actual, typename LRet=Ret>
        LRet call( Actual &/*value*/, const PriorityTag<1> &, typename std::enable_if<std::is_fundamental<LRet>::value, int>::type = 0 ) {
          throw std::runtime_error( "Can't convert" );
        }

        template<typename Actual, typename LRet=Ret, typename W=Wanted, typename std::enable_if<!std::is_same<float, W>::value, int>::type = 0>
        LRet call( Actual &/*value*/, const PriorityTag<0> & ) {
          LRet v;
          if ( ret ) {
            v = *ret;
          } else {
            v = JSONConstructor<Wanted>::construct( static_cast<typename std::remove_cv<typename std::remove_reference<Wanted>::type>::type *>(nullptr) );
          }
          JSONSerialiser<Wanted>::from_json( *j, v );
          return v;
        }

        template<typename X=Wanted> Ret operator()( X &value ) {
          return call( value, PriorityTag<4>() );
        }

      private:
        const JSON *j;
        Ret *ret{ nullptr };
      };
    }

    template<typename T> inline JSON &json_as( T &, JSON &j, JSON * ) {
      return j;
    }

    template<typename T> inline const JSON &json_as( T &, const JSON &j, JSON * ) {
      return j;
    }

    template<typename T>
    typename JSONConstructor<T>::constructType JSON::as() const {
      detail::get_json_value_visitor<typename JSONConstructor<T>::constructType, T> visitor( this );
      return value.apply_visitor( visitor );
    }

    template<typename T>
    typename JSONConstructor<T>::constructType JSON::as( typename JSONConstructor<T>::constructType &other ) const {
      detail::get_json_value_visitor<typename JSONConstructor<T>::constructType, T> visitor( this, &other );
      return value.apply_visitor( visitor );
    }

    template<typename T> inline bool JSON::is() const {
      static_assert( detail::variant_has_type<T, JSON::valueType>::value, "Must be one of the internal types" );
      return value.type() == typeid( T );
    }

    inline JSONPtr JSON::emplaceBack( const JSONPtr &json ) {
      if ( value.type() != typeid( arrayType ) ) {
        value = arrayType();
      }
      assert( json );
      auto &array = boost::get<arrayType>( value );
      array.emplace_back( std::forward<const JSONPtr &>( json ) );
      return *array.rbegin();
    }

    template<typename... T> inline JSONPtr JSON::emplaceBack( T &&...x ) {
      if ( value.type() != typeid( arrayType ) ) {
        value = arrayType();
      }
      auto &array = boost::get<arrayType>( value );
      array.emplace_back( std::make_shared<JSON>( std::forward<T>( x )... ) );
      return *array.rbegin();
    }

    inline size_t JSON::size() const noexcept {
      if ( type() == Type::ARRAY ) {
        return boost::get<arrayType>( value ).size();
      } else {
        return boost::get<objectType>( value ).size();
      }
    }

    inline const JSON::ArrayIterator JSON::arrayIterator() const noexcept {
      return ArrayIterator( &( boost::get<arrayType>( value ) ) );
    }

    inline const JSON::ObjectIterator JSON::objectIterator() const noexcept {
      return ObjectIterator( &( boost::get<objectType>( value ) ) );
    }

    JSONPtr JSON::sub( const keyType &key ) const noexcept {
      JSONPtr rt;
      if ( value.type() == typeid( objectType ) ) {
        auto &obj = boost::get<objectType>( value );
        auto item = obj.find( key );
        if ( item != obj.end() ) {
          rt = item->second;
        }
      }
      return rt;
    }

    template<typename T> JSON &JSON::operator=( const T &v ) {
      JSONSerialiser<T>::to_json( *this, v );
      return *this;
    }

    template<typename T, typename JSONType=JSON, detail::enable_if_t<
      detail::supports_implicit_json_compare<T>::value and std::is_same<JSONType, JSON>::value> = 0>
    auto operator==( const JSONType &j, const T &v ) -> decltype( JSONSerialiser<T>::compare( j, v ) ) {
      return JSONSerialiser<T>::compare( j, v );
    }

    template<typename T, typename JSONType=JSON, detail::enable_if_t<
      detail::supports_implicit_json_compare<T>::value and std::is_same<JSONType, JSON>::value> = 0>
    auto operator!=( const JSONType &j, const T &v ) -> decltype( JSONSerialiser<T>::compare( j, v ) ) {
      return !JSONSerialiser<T>::compare( j, v );
    }
  }
}

#endif //DVJSON_JSONVALUE_H
