//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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

  inline std::size_t erase(const Key & k)
  {
    libMesh::Threads::spin_mutex::scoped_lock lock(spin_mutex);

    return std::unordered_map<Key, T>::erase(k);
  }

  inline bool contains(const Key & key) { return this->find(key) != this->end(); }

private:
  libMesh::Threads::spin_mutex spin_mutex;
};
