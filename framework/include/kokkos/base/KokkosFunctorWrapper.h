//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"

template <typename Object>
class FunctorWrapperHost;

class FunctorWrapperDeviceBase
{
public:
  KOKKOS_FUNCTION FunctorWrapperDeviceBase() {}
  KOKKOS_FUNCTION virtual ~FunctorWrapperDeviceBase() {}

  KOKKOS_FUNCTION virtual void hello() const = 0;
};

template <typename Object>
class FunctorWrapperDevice : public FunctorWrapperDeviceBase
{
  friend class FunctorWrapperHost<Object>;

public:
  KOKKOS_FUNCTION FunctorWrapperDevice() {}

  KOKKOS_FUNCTION void hello() const override final { _functor->hello(); }

private:
  /**
   * Pointer to the functor on device
   */
  Object * _functor = nullptr;
};

class FunctorWrapperHostBase
{
public:
  virtual ~FunctorWrapperHostBase() {}

  /**
   * Allocate device functor and wrapper
   * @returns The pointer to the device wrapper
   */
  virtual FunctorWrapperDeviceBase * allocate() = 0;
  /**
   * Copy functor to device
   */
  virtual void copyFunctor() = 0;
  /**
   * Free host and device copies of functor
   */
  virtual void freeFunctor() = 0;
};

template <typename Object>
class FunctorWrapperHost : public FunctorWrapperHostBase
{
public:
  /**
   * Constructor
   * @param functor Pointer
   */
  FunctorWrapperHost(const void * functor) : _functor_host(*static_cast<const Object *>(functor)) {}
  ~FunctorWrapperHost();

  FunctorWrapperDeviceBase * allocate() final;
  void copyFunctor() final;
  void freeFunctor() final;

private:
  /**
   * Reference of the functor on host
   */
  const Object & _functor_host;
  /**
   * Copy of the functor on host
   */
  std::unique_ptr<Object> _functor_copy;
  /**
   * Copy of the functor on device
   */
  Object * _functor_device = nullptr;
};

template <typename Object>
FunctorWrapperDeviceBase *
FunctorWrapperHost<Object>::allocate()
{
  // Allocate storage for device wrapper on device
  auto wrapper_device = static_cast<FunctorWrapperDevice<Object> *>(
      ::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(FunctorWrapperDevice<Object>)));

  // Allocate device wrapper on device using placement new to populate vtable with device pointers
  ::Kokkos::parallel_for(
      1, KOKKOS_LAMBDA(const int) { new (wrapper_device) FunctorWrapperDevice<Object>(); });

  // Allocate storage for functor on device
  _functor_device =
      static_cast<Object *>(::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(Object)));

  // Let device wrapper point to the copy
  ::Kokkos::Impl::DeepCopy<MemSpace, ::Kokkos::HostSpace>(
      &(wrapper_device->_functor), &_functor_device, sizeof(Object *));

  return wrapper_device;
}

template <typename Object>
void
FunctorWrapperHost<Object>::copyFunctor()
{
  // Make a copy of functor on host to trigger copy constructor
  _functor_copy = std::make_unique<Object>(_functor_host);

  // Copy functor to device
  ::Kokkos::Impl::DeepCopy<MemSpace, ::Kokkos::HostSpace>(
      _functor_device, _functor_copy.get(), sizeof(Object));
}

template <typename Object>
void
FunctorWrapperHost<Object>::freeFunctor()
{
  _functor_copy.reset();
}

template <typename Object>
FunctorWrapperHost<Object>::~FunctorWrapperHost()
{
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(_functor_device);
}
