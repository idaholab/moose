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
 * functionality for computing derivatives of the temperature
 * variable.
 *
 * This class is templated so that it can be used by either
 * a Kernel object or a BC object.
 */
template <class T>
class NSTemperatureDerivs
{
public:
  NSTemperatureDerivs(T & x);

  /**
   * The primary interfaces for computing temperature derivatives.
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
NSTemperatureDerivs<T>::NSTemperatureDerivs(T & x) : _data(x)
{
}

template <class T>
Real
NSTemperatureDerivs<T>::get_grad(unsigned i)
{
  // Convenience vars
  const Real U0 = _data._rho[_data._qp];
  const Real U1 = _data._rho_u[_data._qp];
  const Real U2 = _data._rho_v[_data._qp];
  const Real U3 = _data._rho_w[_data._qp];
  const Real U4 = _data._rho_et[_data._qp];

  const Real rho2 = U0 * U0;
  const Real mom2 = U1 * U1 + U2 * U2 + U3 * U3;
  const Real tmp = -1.0 / rho2 / _data._fp.cv();

  switch (i)
  {
    case 0: // dT/d(rho)
      return (U4 - (mom2 / U0)) * tmp;

    case 1: // dT/d(rho*u)
      return U1 * tmp;

    case 2: // dT/d(rho*v)
      return U2 * tmp;

    case 3: // dT/d(rho*w)
      return U3 * tmp;

    case 4: // dT/d(rho*e)
      return -U0 * tmp;

    default:
      mooseError("Should not get here!");
      break;
  }
}

template <class T>
Real
NSTemperatureDerivs<T>::get_hess(unsigned i, unsigned j)
{
  // Convenience vars
  const Real U0 = _data._rho[_data._qp];
  const Real U1 = _data._rho_u[_data._qp];
  const Real U2 = _data._rho_v[_data._qp];
  const Real U3 = _data._rho_w[_data._qp];
  const Real U4 = _data._rho_et[_data._qp];

  const Real rho2 = U0 * U0;
  const Real rho3 = rho2 * U0;
  const Real rho4 = rho3 * U0;
  const Real mom2 = U1 * U1 + U2 * U2 + U3 * U3;

  const Real cv = _data._fp.cv();
  const Real tmp = -1.0 / rho2 / cv;

  // Only lower-triangle of matrix is defined, it is symmetric
  if (i < j)
    std::swap(i, j);

  // Map (i,j) into row-major storage index, 5 entries per row
  unsigned idx = 5 * i + j;

  switch (idx)
  {
    // Row 0
    case 0: // rho, rho derivative
      return 2.0 * U4 / rho3 / cv - 3.0 * mom2 / rho4 / cv;

    // Row 1
    case 5: // rho*u, rho
      return 2.0 * U1 / rho3 / cv;

    case 6: // rho*u, rho*u
      return tmp;

    // Row 2
    case 10: // rho*v, rho
      return 2.0 * U2 / rho3 / cv;

    case 12: // rho*v, rho*v
      return tmp;

    // Row 3
    case 15: // rho*w, rho
      return 2.0 * U3 / rho3 / cv;

    case 18: // rho*w, rho*w
      return tmp;

    // Row 4
    case 20: // rho*e, rho
      return tmp;

    case 11:
    case 16:
    case 17:
    case 21:
    case 22:
    case 23:
    case 24:
      return 0.0;

    default:
      mooseError("Should not get here!");
      break;
  }

  return 0.0;
}
