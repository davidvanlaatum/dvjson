#pragma once
#ifndef DVFORMSCPP_UNORDEREDINDEXEDMAP_H
#define DVFORMSCPP_UNORDEREDINDEXEDMAP_H

#include <vector>
#include <map>

namespace dv {
  namespace json {
    template<typename Key, typename T, class Compare = std::less<Key>,
             class Allocator = std::allocator<std::pair<const Key, T>>,
             class IndexAllocator = std::allocator<std::pair<const Key, typename std::vector<Key>::size_type>>>
      class UnorderedIndexedMap {
       public:
        typedef Key key_type;
        typedef T mapped_type;
        typedef std::pair<const key_type, mapped_type> value_type;
        typedef Compare key_compare;
        typedef Allocator allocator_type;
        typedef typename allocator_type::reference reference;
        typedef typename allocator_type::const_reference const_reference;
        typedef typename allocator_type::pointer pointer;
        typedef typename allocator_type::const_pointer const_pointer;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::difference_type difference_type;
        typedef std::vector<value_type, Allocator> valuesType;
        typedef std::map<Key, typename valuesType::size_type, Compare, IndexAllocator> indexType;
        typedef UnorderedIndexedMap<Key, T, Compare, Allocator, IndexAllocator> self;

        template<typename V> class iteratorTmpl {
         public:
          typedef iteratorTmpl<V> iteratorType;
          typedef V value_type;
          typedef ptrdiff_t difference_type;
          typedef value_type *pointer;
          typedef value_type &reference;
          typedef std::input_iterator_tag iterator_category;
          typedef typename std::conditional<std::is_const<value_type>::value, const self &, self &>::type selfType;

          iteratorTmpl( size_type nIndex, selfType &nMap, bool nFind = false ) : index( nIndex ), map( nMap ), find( nFind ) {}

          iteratorType &operator++() {
            if ( find ) {
              index = std::numeric_limits<size_type>::max();
            } else {
              index = map.size() > index + 1 ? index + 1 : std::numeric_limits<size_type>::max();
            }
            return *this;
          }

          bool operator==( const iteratorType &other ) const { return index == other.index && &map == &other.map; }

          bool operator!=( const iteratorType &other ) const { return !( *this == other ); }

          reference operator*() const { return map.values.at( index ); }

          pointer operator->() const { return &map.values.at( index ); }

         protected:
          size_type index;
          selfType map;
          bool find;
        };

        typedef iteratorTmpl<self::value_type> iterator;
        typedef iteratorTmpl<const self::value_type> const_iterator;

//        static_assert(std::is_const<typename const_iterator::value_type>::value,"bla");
//        static_assert(std::is_const<typename const_iterator::reference >::value,"bla");

//        typename decltype( *std::declval<const_iterator>() )::X y;
//        static_assert( std::is_const<decltype( *std::declval<const_iterator>() )>::value, "bla" );

//        typedef typename const_iterator::reference::X y;

        /*class const_iterator {
         public:
          typedef const self::value_type value_type;
          typedef ptrdiff_t difference_type;
          typedef value_type *pointer;
          typedef value_type &reference;
          typedef std::input_iterator_tag iterator_category;

          const_iterator( size_t nIndex, const self &nMap, bool nFind = false ) : index( nIndex ), map( nMap ), find( nFind ) {}

          const_iterator &operator++() {
            index = map.size() > index ? index + 1 : std::numeric_limits<size_type>::max();
            return *this;
          }

          bool operator==( const_iterator other ) const { return index == other.index && &map == &other.map; }

          bool operator!=( const_iterator other ) const { return !( *this == other ); }

          reference operator*() const { return map.values.at( index ); }

          pointer operator->() const { return map.values.at( index ); }

         protected:
          size_type index;
          const self &map;
          bool find;
        };*/

        UnorderedIndexedMap() = default;
        UnorderedIndexedMap( const UnorderedIndexedMap & ) = default;
        UnorderedIndexedMap( UnorderedIndexedMap && ) = default;
        ~UnorderedIndexedMap() = default;

        inline self &operator=( const self &other ) {
          for ( const auto &item : other.values ) {
            emplace( item.first, item.second );
          }
          return *this;
        }

        inline self &operator=( self &&other ) noexcept {
          index = std::move( other.index );
          values = std::move( other.values );
          return *this;
        }

        inline bool operator==( const self &other ) const {
          return this == &other;
        }

        inline bool operator!=( const self &other ) const {
          return this != &other;
        }

        inline size_type size() const noexcept { return values.size(); }

        inline iterator begin() noexcept { return iterator( empty() ? std::numeric_limits<size_type>::max() : 0, *this ); }

        inline iterator end() noexcept { return iterator( std::numeric_limits<size_type>::max(), *this ); }

        inline const_iterator begin() const noexcept { return const_iterator( empty() ? std::numeric_limits<size_type>::max() : 0, *this ); }

        inline const_iterator end() const noexcept { return const_iterator( std::numeric_limits<size_type>::max(), *this ); }

        inline bool empty() const noexcept { return values.empty(); }

        inline const_iterator find( const key_type &key ) const {
          const auto &item = index.find( key );
          if ( item == index.end() ) {
            return const_iterator( std::numeric_limits<size_type>::max(), *this );
          } else {
            return const_iterator( item->second, *this, true );
          }
        }

        inline mapped_type &operator[]( const key_type &key ) {
          size_type i;
          const auto &it = index.find( key );
          if ( it == index.end() ) {
            i = values.size();
            index.emplace( key, i );
            values.emplace_back( key, mapped_type() );
            return values.back().second;
          } else {
            return values[it->second].second;
          }
        }

        template<class... Args> iterator emplace( const Key &key, Args &&... args ) {
          size_type i;
          const auto &it = index.find( key );
          if ( it == index.end() ) {
            i = values.size();
            values.emplace_back( key, std::forward<Args>( args )... );
            index.emplace( key, i );
          } else {
            i = it->second;
            values[it->second].second = mapped_type( std::forward<Args>( args )... );
          }
          return iterator( i, *this );
        }

       protected:
        valuesType values;
        indexType index;
      };
  }
};

#endif //DVFORMSCPP_UNORDEREDINDEXEDMAP_H
