//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// FluidProperties includes
#include "IdealGasFluidProperties.h"

/**
 * Class outside the Moose hierarchy that contains common
 * functionality for computing derivatives of the pressure
 * variable.
 *
 * This class is templated so that it can be used by either
 * a Kernel object or a BC object.
 */
template <class T>
class NSPressureDerivs
{
public:
  NSPressureDerivs(T & x);

  /**
   * The primary interfaces for computing pressure derivatives.
   * Requires access to protected values from the _data reference.
   * The indices input to these functions are in terms of the
   * "canonical" variable numbering.
   */
  Real get_grad(unsigned i);
  Real get_hess(unsigned i, unsigned j);

private:
  T & _data;
};

template <class T>
NSPressureDerivs<T>::NSPressureDerivs(T & x) : _data(x)
{
}

template <class T>
Real
NSPressureDerivs<T>::get_grad(unsigned i)
{
  // Convenience vars
  const Real u = _data._u_vel[_data._qp];
  const Real v = _data._v_vel[_data._qp];
  const Real w = _data._w_vel[_data._qp];

  const Real vel2 = (u * u + v * v + w * w);
  const Real gam = _data._fp.gamma();

  switch (i)
  {
    case 0: // dP/d(rho)
      return 0.5 * (gam - 1.0) * vel2;

    case 1: // dP/d(rho*u)
      return (1.0 - gam) * u;

    case 2: // dP/d(rho*v)
      return (1.0 - gam) * v;

    case 3: // dP/d(rho*w)
      return (1.0 - gam) * w;

    case 4: // dP/d(rho*e)
      return gam - 1.;

    default:
      mooseError("Should not get here!");
      break;
  }
}

template <class T>
Real
NSPressureDerivs<T>::get_hess(unsigned i, unsigned j)
{
  // Convenience variables
  const Real U0 = _data._rho[_data._qp];

  const Real u = _data._u_vel[_data._qp];
  const Real v = _data._v_vel[_data._qp];
  const Real w = _data._w_vel[_data._qp];
  const Real vel2 = (u * u + v * v + w * w);

  // Save some typing...
  const Real gam = _data._fp.gamma();

  // A frequently-used variable
  const Real tmp = (1.0 - gam) / U0;

  // Only lower-triangle of matrix is defined, it is symmetric
  if (i < j)
    std::swap(i, j);

  // Map (i,j) into row-major storage index, 5 entries per row
  const unsigned int idx = 5 * i + j;

  switch (idx)
  {
    // Row 0
    case 0: // rho, rho derivative
      return tmp * vel2;

    // Row 1
    case 5: // rho*u, rho
      return -tmp * u;

    case 6: // rho*u, rho*u
      return tmp;

    // Row 2
    case 10: // rho*v, rho
      return -tmp * v;

    case 12: // rho*v, rho*v
      return tmp;

    // Row 3
    case 15: // rho*w, rho
      return -tmp * w;

    case 18: // rho*w, rho*w
      return tmp;

    case 11:
    case 16:
    case 17:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      return 0.;

    default:
      mooseError("Should not get here!");
      break;
  }

  return 0.0;
}
