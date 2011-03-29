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

/// HashMap is an abstraction for dictionary-type data, we are using thread-safe version when we have TBB and std::map if not

//#ifdef LIBMESH_HAVE_TBB_API
//// threaded
//#include "tbb/concurrent_hash_map.h"
//
//template<typename Key, typename T, typename HashCompare = tbb_hash_compare<Key>, typename Allocator = tbb_allocator<std::pair<Key, T> > >
//class HashMap : public tbb::concurrent_hash_map< Key, T, HashCompare, Allocator >
//{
//};
//
//#else
// serial
#include <map>
#include <functional>

template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key,T> > >
class HashMap : public std::map< Key, T, Compare, Allocator >
{
};

//#endif

#endif /* HASHMAP_H */
