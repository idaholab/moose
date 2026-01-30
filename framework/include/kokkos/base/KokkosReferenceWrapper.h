//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"

#include "MooseTypes.h"

namespace Moose::Kokkos
{

/**
 * The Kokkos object that can hold the reference of a variable.
 * Reference of a host variable is not accessible on device, so if there is a variable that should
 * be stored as a reference but still needs to be accessed on device, define an instance of this
 * class and construct it with the reference of the variable.
 * This class holds the device copy as well as the host reference of the variable.
 * The copy constructor of this object that copies the host reference to the device copy is invoked
 * whenever a Kokkos functor containing this object is dispatched to device, so it is guaranteed
 * that the device copy is always up-to-date with the host reference when it is used on device
 * Therefore, the variable must be copy constructible.
 */
template <typename T>
class ReferenceWrapper
{
public:
  /**
   * Constructor
   * @param reference The writeable reference of the variable to store
   */
  ReferenceWrapper(T & reference) : _reference(reference), _copy(reference) {}
  /**
   * Copy constructor
   */
  ReferenceWrapper(const ReferenceWrapper<T> & object)
    : _reference(object._reference), _copy(object._reference)
  {
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the const reference of the stored variable
   * @returns The const reference of the stored variable depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION operator const T &() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)

    return _copy;
  }
  /**
   * Get the const reference of the stored variable
   * @returns The const reference of the stored variable depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const T & operator*() const
  {
    KOKKOS_IF_ON_HOST(return _reference;)

    return _copy;
  }
  /**
   * Get the const pointer to the stored variable
   * @returns The const pointer to the stored variable depending on the architecture this function
   * is being called on
   */
  KOKKOS_FUNCTION const T * operator->() const
  {
    KOKKOS_IF_ON_HOST(return &_reference;)

    return &_copy;
  }
  /**
   * Forward arguments to the stored variable's const operator() depending on the architecture this
   * function is being called on
   * @param args The variadic arguments to be forwarded
   */
  template <typename... Args>
  KOKKOS_FUNCTION auto operator()(Args &&... args) const -> decltype(auto)
  {
    KOKKOS_IF_ON_HOST(return _reference(std::forward<Args>(args)...);)

    return _copy(std::forward<Args>(args)...);
  }
#else
  /**
   * Get the const reference of the stored host reference
   * @returns The const reference of the stored host reference
   */
  operator const T &() const { return _reference; }
  /**
   * Get the const reference of the stored host reference
   * @returns The const reference of the stored host reference
   */
  const T & operator*() const { return _reference; }
  /**
   * Get the const pointer of the stored host reference
   * @returns The const pointer to the stored host reference
   */
  const T * operator->() const { return &_reference; }
  /**
   * Forward arguments to the stored host reference's const operator()
   * @param args The variadic arguments to be forwarded
   */
  template <typename... Args>
  auto operator()(Args &&... args) const -> decltype(auto)
  {
    return _reference(std::forward<Args>(args)...);
  }
#endif
  /**
   * Get the writeable reference of the stored host reference
   * @returns The writeable reference of the stored host reference
   */
  operator T &() { return _reference; }
  /**
   * Get the writeable reference of the stored host reference
   * @returns The writeable reference of the stored host reference
   */
  T & operator*() { return _reference; }
  /**
   * Get the writeable pointer of the stored host reference
   * @returns The writeable pointer to the stored host reference
   */
  T * operator->() { return &_reference; }
  /**
   * Forward arguments to the stored host reference's operator()
   * @param args The variadic arguments to be forwarded
   */
  template <typename... Args>
  auto operator()(Args &&... args) -> decltype(auto)
  {
    return _reference(std::forward<Args>(args)...);
  }

protected:
  /**
   * Writeable host reference of the variable
   */
  T & _reference;
  /**
   * Device copy of the variable
   */
  const T _copy;
};

template <typename T>
struct ArrayDeepCopy<ReferenceWrapper<T>>
{
  static constexpr bool value = true;
};

} // namespace Moose::Kokkos
