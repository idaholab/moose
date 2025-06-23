//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_GPU_SCOPE
#include "GPUHeader.h"
#endif

#include "MooseTypes.h"

template <typename T>
class GPUReferenceWrapper
{
protected:
  // Reference
  T & _reference;
  // Copy
  T _copy;

public:
  GPUReferenceWrapper(T & reference) : _reference(reference) {}
  GPUReferenceWrapper(const GPUReferenceWrapper<T> & object) : _reference(object._reference)
  {
    _copy = object._reference;
  }

#ifdef MOOSE_GPU_SCOPE
  KOKKOS_FUNCTION operator const T &() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)
    KOKKOS_IF_ON_DEVICE(return _copy;)
  }
  KOKKOS_FUNCTION const T & operator*() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)
    KOKKOS_IF_ON_DEVICE(return _copy;)
  }
  KOKKOS_FUNCTION const T * operator->() const
  {
    KOKKOS_IF_ON_HOST(return &_reference;)
    KOKKOS_IF_ON_DEVICE(return &_copy;)
  }

  template <typename... Args>
  KOKKOS_FUNCTION auto operator()(Args &&... args) const -> decltype(auto)
  {
    KOKKOS_IF_ON_HOST(return _reference(std::forward<Args>(args)...);)
    KOKKOS_IF_ON_DEVICE(return _copy(std::forward<Args>(args)...);)
  }
#else
  operator const T &() const { return _reference; }
  const T & operator*() const { return _reference; }
  const T * operator->() const { return &_reference; }

  template <typename... Args>
  auto operator()(Args &&... args) const -> decltype(auto)
  {
    return _reference(std::forward<Args>(args)...);
  }
#endif

  operator T &() { return _reference; }
  T & operator*() { return _reference; }
  T * operator->() { return &_reference; }

  template <typename... Args>
  auto operator()(Args &&... args) -> decltype(auto)
  {
    return _reference(std::forward<Args>(args)...);
  }
};

template <typename T>
class GPUReferenceWrapper<const T>
{
protected:
  // Reference
  const T & _reference;
  // Copy
  T _copy;

public:
  GPUReferenceWrapper(const T & reference) : _reference(reference) {}
  GPUReferenceWrapper(const GPUReferenceWrapper<const T> & object) : _reference(object._reference)
  {
    _copy = object._reference;
  }

#ifdef MOOSE_GPU_SCOPE
  KOKKOS_FUNCTION operator const T &() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)
    KOKKOS_IF_ON_DEVICE(return _copy;)
  }
  KOKKOS_FUNCTION const T & operator*() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)
    KOKKOS_IF_ON_DEVICE(return _copy;)
  }
  KOKKOS_FUNCTION const T * operator->() const
  {
    KOKKOS_IF_ON_HOST(return &_reference;)
    KOKKOS_IF_ON_DEVICE(return &_copy;)
  }

  template <typename... Args>
  KOKKOS_FUNCTION auto operator()(Args &&... args) const -> decltype(auto)
  {
    KOKKOS_IF_ON_HOST(return _reference(std::forward<Args>(args)...);)
    KOKKOS_IF_ON_DEVICE(return _copy(std::forward<Args>(args)...);)
  }
#else
  operator const T &() const { return _reference; }
  const T & operator*() const { return _reference; }
  const T * operator->() const { return &_reference; }

  template <typename... Args>
  auto operator()(Args &&... args) const -> decltype(auto)
  {
    return _reference(std::forward<Args>(args)...);
  }
#endif
};
