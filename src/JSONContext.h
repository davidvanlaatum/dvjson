#pragma once
#ifndef DVFORMSCPP_JSONCONTEXT_H
#define DVFORMSCPP_JSONCONTEXT_H

#include "jsonfwd.h"   // for PriorityTag, PURE
#include <functional>  // for function
#include <map>         // for map, _Rb_tree_iterator
#include <memory>      // for shared_ptr, make_shared
#include <typeinfo>    // for type_info
#include <utility>     // for pair

namespace dv {
  namespace json {
    class JSONContext {
      struct ContextRestore {
        ContextRestore() = default;
        ContextRestore( ContextRestore && ) = default;
        ~ContextRestore();
      };
     public:
      template<class T> inline std::shared_ptr<T> get() PURE;
      template<class T> inline auto makeDefault( PriorityTag<1> ) const -> decltype( std::make_shared<typename T::defaultContextType>() );
      template<class T> inline std::shared_ptr<T> makeDefault( PriorityTag<0> ) const;
      template<class T> inline void attach( T *, bool takeOwnership = true );
      template<class T> inline void attach( const std::shared_ptr<T> & );
      template<typename T> inline void attach( const T & );
      inline static JSONContext *current() PURE;
      void enter( const std::function<void()> &function );
      JSONContext::ContextRestore enter();
     protected:
      std::map<const std::type_info *, std::shared_ptr<void>> items;
      static thread_local JSONContext *currentContext;
      JSONContext *previous{ nullptr };
    };

    template<class T>
    std::shared_ptr<T> JSONContext::get() {
      std::shared_ptr<T> rt;
      auto it = items.find( &typeid( T ) );
      if ( it != items.end() ) {
        rt = std::static_pointer_cast<T>( it->second );
      } else if ( previous ) {
        rt = previous->get<T>();
      }
      if ( !rt ) {
        rt = makeDefault<T>( PriorityTag<1>() );
        if ( rt ) {
          items[&typeid( T )] = rt;
        }
      }
      return rt;
    }

    template<class T>
    auto JSONContext::makeDefault( PriorityTag<1> ) const -> decltype( std::make_shared<typename T::defaultContextType>() ) {
      return std::make_shared<typename T::defaultContextType>();
    }

    template<class T> inline std::shared_ptr<T> JSONContext::makeDefault( PriorityTag<0> ) const {
      return std::shared_ptr<T>();
    }

    template<typename T> struct NullDeleter {
      void operator()( T * ) {}
    };

    template<class T>
    void JSONContext::attach( T *obj, bool takeOwnership ) {
      if ( takeOwnership ) {
        items[&typeid( T )] = std::shared_ptr<T>( obj );
      } else {
        items[&typeid( T )] = std::shared_ptr<T>( obj, NullDeleter<T>() );
      }
    }

    template<typename T>
    void JSONContext::attach( const T &obj ) {
      attach( std::make_shared<T>( obj ) );
    }

    template<class T>
    void JSONContext::attach( const std::shared_ptr<T> &ptr ) {
      items[&typeid( T )] = ptr;
    }

    JSONContext *JSONContext::current() {
      return currentContext;
    }
  }
}

#endif //DVFORMSCPP_JSONCONTEXT_H
