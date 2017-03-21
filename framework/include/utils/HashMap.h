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

/// HashMap is an abstraction for dictionary data type, we make it thread-safe by locking inserts
#include "libmesh/threads.h"

#include <unordered_map>

template <typename Key, typename T>
class HashMap : public std::unordered_map<Key, T>
{
public:
  inline T & operator[](const Key & k)
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(spin_mutex);

    return std::unordered_map<Key, T>::operator[](k);
  }

  inline bool contains(const Key & key) { return this->find(key) != this->end(); }

private:
  libMesh::Threads::spin_mutex spin_mutex;
};

#endif /* HASHMAP_H */
