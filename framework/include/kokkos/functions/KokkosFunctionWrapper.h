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

namespace Moose
{
namespace Kokkos
{

template <typename Object>
class FunctionWrapperHost;

class FunctionWrapperDeviceBase
{
public:
  KOKKOS_FUNCTION FunctionWrapperDeviceBase() {}
  KOKKOS_FUNCTION virtual ~FunctionWrapperDeviceBase() {}

  KOKKOS_FUNCTION virtual Real value(Real t, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real3 gradient(Real t, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real3 curl(Real t, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real div(Real t, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real timeDerivative(Real t, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real timeIntegral(Real t1, Real t2, Real3 p) const = 0;
  KOKKOS_FUNCTION virtual Real integral() const = 0;
  KOKKOS_FUNCTION virtual Real average() const = 0;
};

template <typename Object>
class FunctionWrapperDevice : public FunctionWrapperDeviceBase
{
  friend class FunctionWrapperHost<Object>;

public:
  KOKKOS_FUNCTION FunctionWrapperDevice() {}

  KOKKOS_FUNCTION Real value(Real t, Real3 p) const override final
  {
    return _function->value(t, p);
  }
  KOKKOS_FUNCTION Real3 gradient(Real t, Real3 p) const override final
  {
    return _function->gradient(t, p);
  }
  KOKKOS_FUNCTION Real3 curl(Real t, Real3 p) const override final { return _function->curl(t, p); }
  KOKKOS_FUNCTION Real div(Real t, Real3 p) const override final { return _function->div(t, p); }
  KOKKOS_FUNCTION Real timeDerivative(Real t, Real3 p) const override final
  {
    return _function->timeDerivative(t, p);
  }
  KOKKOS_FUNCTION Real timeIntegral(Real t1, Real t2, Real3 p) const override final
  {
    return _function->timeIntegral(t1, t2, p);
  }
  KOKKOS_FUNCTION Real integral() const override final { return _function->integral(); }
  KOKKOS_FUNCTION Real average() const override final { return _function->average(); }

protected:
  /**
   * Pointer to the function on device
   */
  Object * _function = nullptr;
};

class FunctionWrapperHostBase
{
public:
  virtual ~FunctionWrapperHostBase() {}

  /**
   * Allocate device function and wrapper
   * @returns The pointer to the device wrapper
   */
  virtual FunctionWrapperDeviceBase * allocate() = 0;
  /**
   * Copy function to device
   */
  virtual void copyFunction() = 0;
  /**
   * Free host and device copies of function
   */
  virtual void freeFunction() = 0;
};

template <typename Object>
class FunctionWrapperHost : public FunctionWrapperHostBase
{
public:
  /**
   * Constructor
   * @param function Pointer to the function
   */
  FunctionWrapperHost(const void * function)
    : _function_host(*static_cast<const Object *>(function))
  {
  }
  ~FunctionWrapperHost();

  FunctionWrapperDeviceBase * allocate() final;
  void copyFunction() final;
  void freeFunction() final;

private:
  /**
   * Reference of the function on host
   */
  const Object & _function_host;
  /**
   * Copy of the function on host
   */
  std::unique_ptr<Object> _function_copy;
  /**
   * Copy of the function on device
   */
  Object * _function_device = nullptr;
};

template <typename Object>
FunctionWrapperDeviceBase *
FunctionWrapperHost<Object>::allocate()
{
  // Allocate storage for device wrapper on device
  auto wrapper_device = static_cast<FunctionWrapperDevice<Object> *>(
      ::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(FunctionWrapperDevice<Object>)));

  // Allocate device wrapper on device using placement new to populate vtable with device pointers
  ::Kokkos::parallel_for(
      1, KOKKOS_LAMBDA(const int) { new (wrapper_device) FunctionWrapperDevice<Object>(); });

  // Allocate storage for function on device
  _function_device =
      static_cast<Object *>(::Kokkos::kokkos_malloc<ExecSpace::memory_space>(sizeof(Object)));

  // Let device wrapper point to the copy
  ::Kokkos::Impl::DeepCopy<MemSpace, ::Kokkos::HostSpace>(
      &(wrapper_device->_function), &_function_device, sizeof(Object *));

  return wrapper_device;
}

template <typename Object>
void
FunctionWrapperHost<Object>::copyFunction()
{
  // Make a copy of function on host to trigger copy constructor
  _function_copy = std::make_unique<Object>(_function_host);

  // Copy function to device
  ::Kokkos::Impl::DeepCopy<MemSpace, ::Kokkos::HostSpace>(
      _function_device, _function_copy.get(), sizeof(Object));
}

template <typename Object>
void
FunctionWrapperHost<Object>::freeFunction()
{
  _function_copy.reset();
}

template <typename Object>
FunctionWrapperHost<Object>::~FunctionWrapperHost()
{
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(_function_device);
}

} // namespace Kokkos
} // namespace Moose
