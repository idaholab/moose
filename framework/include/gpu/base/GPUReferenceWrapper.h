//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_SCOPE
#include "GPUHeader.h"
#endif

#include "MooseTypes.h"

namespace Moose
{
namespace Kokkos
{

template <typename T>
class ReferenceWrapper
{
protected:
  // Reference
  T & _reference;
  // Copy
  T _copy;

public:
  ReferenceWrapper(T & reference) : _reference(reference) {}
  ReferenceWrapper(const ReferenceWrapper<T> & object) : _reference(object._reference)
  {
    _copy = object._reference;
  }

#ifdef MOOSE_KOKKOS_SCOPE
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
class ReferenceWrapper<const T>
{
protected:
  // Reference
  const T & _reference;
  // Copy
  T _copy;

public:
  ReferenceWrapper(const T & reference) : _reference(reference) {}
  ReferenceWrapper(const ReferenceWrapper<const T> & object) : _reference(object._reference)
  {
    _copy = object._reference;
  }

#ifdef MOOSE_KOKKOS_SCOPE
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

} // namespace Kokkos
} // namespace Moose
