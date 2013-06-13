/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef HASHMAP_H
#define HASHMAP_H

/// HashMap is an abstraction for dictionary data type, we are using thread-safe version when we have TBB and std::map if not

#ifdef LIBMESH_HAVE_TBB_API

// TBB has some very annoying unused-parameter warnings.  I'm disabling them here with pragmas for common compilers
#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__)
#pragma GCC system_header
#endif

// thread-safe container
#include "tbb/concurrent_hash_map.h"

template<typename Key, typename T, typename HashCompare = tbb::tbb_hash_compare<Key>, typename Allocator = tbb::tbb_allocator<std::pair<Key, T> > >
class HashMap : public tbb::concurrent_hash_map< Key, T, HashCompare, Allocator >
{
public:

    inline T & operator[](const Key & key)
    {
      typename tbb::concurrent_hash_map< Key, T, HashCompare, Allocator >::accessor accessor;
      this->insert(accessor, std::make_pair(key,T()));
      return accessor->second;
    }

};
#else

// STL container
#include <map>
#include <functional>

template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key,T> > >
class HashMap : public std::map< Key, T, Compare, Allocator >
{
};

#endif

#endif /* HASHMAP_H */


/**
 * Note: We might decide to use the unordered map instead.  It has a operator[] defined on it.
 *       This declaration should work.
 */
//#include "tbb/concurrent_unordered_map.h"
//
//template<typename Key, typename T, typename HashCompare = tbb::tbb_hash<Key>, typename Equal = std::equal_to<Key>,
//  typename Allocator = tbb::tbb_allocator<std::pair<Key, T> > >
//class HashMap : public tbb::concurrent_unordered_map< Key, T, HashCompare, Equal, Allocator >
