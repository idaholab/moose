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

/**
 * The base class for a user to derive their own Kokkos functions.
 *
 * The user should define the hook methods in their derived class as inlined public methods (not
 * virtual override) with the same signature. If they are defined in the derived class, they will
 * hide the default definitions in the base class. However, the default definitions are not to be
 * actually called. If a hook method was not defined in the derived class, it should not be called.
 */
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
   * Evaluate a scalar value at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The scalar value evaluated at the time and location
   */
  KOKKOS_FUNCTION Real value(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   * Evaluate a vector value at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The vector value evaluated at the time and location
   */
  KOKKOS_FUNCTION Real3 vectorValue(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return Real3(0);
  }
  /**
   * Evaluate a gradient at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The gradient evaluated at the time and location
   */
  KOKKOS_FUNCTION Real3 gradient(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return Real3(0);
  }
  /**
   * Evaluate a curl at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The curl evaluated at the time and location
   */
  KOKKOS_FUNCTION Real3 curl(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return Real3(0);
  }
  /**
   * Evaluate a divergence at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The divergence evaluated at the time and location
   */
  KOKKOS_FUNCTION Real div(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   * Evaluate a time derivative at point (t,x,y,z)
   * @param t The time
   * @param p The location in space (x,y,z)
   * @returns The time derivative evaluated at the time and location
   */
  KOKKOS_FUNCTION Real timeDerivative(Real /* t */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   * Evaluate a time integral at point (x,y,z) between time \p t1 and \p t2
   * @param t1 The beginning time
   * @param t2 The end time
   * @param p The location in space (x,y,z)
   * @returns The time integral evaluated at the location between the times
   */
  KOKKOS_FUNCTION Real timeIntegral(Real /* t1 */, Real /* t2 */, Real3 /* p */) const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   * Evaluate the integral over the domain
   * @returns The integral over the domain
   */
  KOKKOS_FUNCTION Real integral() const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
  /**
   * Evaluate the average over the domain
   * @returns The average over the domain
   */
  KOKKOS_FUNCTION Real average() const
  {
    KOKKOS_ASSERT(false);
    return 0;
  }
};

/**
 * The abstract class that provides polymorphic interfaces for a function.
 *
 * NOTE: This class is not the base class for a Kokkos function derivation. The user should derive
 * their own function from Moose::Kokkos::FunctionBase.
 */
class Function final
{
public:
  /**
   * Constructor
   * @param wrapper The host function wrapper
   */
  Function(std::shared_ptr<FunctionWrapperHostBase> wrapper);
  /**
   * Copy constructor for parallel dispatch
   */
  Function(const Function & function);
  /**
   * Destructor
   */
  ~Function();

  KOKKOS_FUNCTION Real value(Real t, Real3 p) const { return _wrapper_device->value(t, p); }
  KOKKOS_FUNCTION Real3 vectorValue(Real t, Real3 p) const
  {
    return _wrapper_device->vectorValue(t, p);
  }
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
