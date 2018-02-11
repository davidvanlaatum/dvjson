#pragma once
#ifndef DVJSON_JSONOBJECT_H
#define DVJSON_JSONOBJECT_H

#include "jsonfwd.h"
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

namespace dv {
  namespace json {
    class JSONObject {
     public:
      typedef std::string keyType;
      typedef std::pair<keyType, JSONPtr> valueType;
      typedef std::vector<valueType> containerType;
      typedef std::map<const keyType, containerType::size_type> indexType;
      typedef containerType::iterator iterator;
      typedef containerType::const_iterator const_iterator;

      class findIterator {
       public:
        explicit findIterator( const valueType *ptr );
        const valueType *operator->() const;
        bool operator==( const const_iterator &other ) const;
        bool operator!=( const const_iterator &other ) const;
       protected:
        const valueType *ptr;
      };

      JSONObject() = default;
      JSONObject( const JSONObject & ) = default;
      ~JSONObject() = default;

      JSONObject &operator=( const JSONObject & );

      const_iterator begin() const;
      const_iterator end() const;
      typedef containerType::size_type size_type;
      size_type size() const;
      bool empty() const;
      findIterator find( const keyType & ) const;
      JSONPtr &operator[]( const keyType & );
      bool operator==( const JSONObject &other ) const;
      bool operator!=( const JSONObject &other ) const;
     protected:
      containerType container;
      indexType index;
    };
  }
}

#endif //DVJSON_JSONOBJECT_H
