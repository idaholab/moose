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
#include "KokkosFunctionWrapper.h"
#include "KokkosFunctorRegistry.h"

#include "FunctionBase.h"

namespace Moose
{
namespace Kokkos
{

class FunctionBase : public Moose::FunctionBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  FunctionBase(const InputParameters & parameters);
  /**
   * Copy constructor for parallel dispatch
   */
  FunctionBase(const FunctionBase & object);

  /**
   *
   */
  KOKKOS_FUNCTION Real value(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real3 gradient(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return Real3(0);
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real3 curl(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return Real3(0);
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real div(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real timeDerivative(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real timeIntegral(Real /* t1 */, Real /* t2 */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real integral() const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   *
   */
  KOKKOS_FUNCTION Real average() const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
};

class Function
{
public:
  Function(std::shared_ptr<FunctionWrapperHostBase> wrapper);
  Function(const Function & function);
  ~Function();

  KOKKOS_FUNCTION Real value(Real t, Real3 p) const { return _wrapper_device->value(t, p); }
  KOKKOS_FUNCTION Real3 gradient(Real t, Real3 p) const { return _wrapper_device->gradient(t, p); }
  KOKKOS_FUNCTION Real3 curl(Real t, Real3 p) const { return _wrapper_device->curl(t, p); }
  KOKKOS_FUNCTION Real div(Real t, Real3 p) const { return _wrapper_device->div(t, p); }
  KOKKOS_FUNCTION Real timeDerivative(Real t, Real3 p) const
  {
    return _wrapper_device->timeDerivative(t, p);
  }
  KOKKOS_FUNCTION Real timeIntegral(Real t1, Real t2, Real3 p) const
  {
    return _wrapper_device->timeIntegral(t1, t2, p);
  }
  KOKKOS_FUNCTION Real integral() const { return _wrapper_device->integral(); }
  KOKKOS_FUNCTION Real average() const { return _wrapper_device->average(); }

private:
  /**
   * Pointer to the host function wrapper
   */
  std::shared_ptr<FunctionWrapperHostBase> _wrapper_host;
  /**
   * Pointer to the device function wrapper
   */
  FunctionWrapperDeviceBase * _wrapper_device = nullptr;
};

}
}
