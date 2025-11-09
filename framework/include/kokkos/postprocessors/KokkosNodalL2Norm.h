//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalSum.h"

/**
 * Computes the "nodal" L2-norm of the coupled variable, which is
 * defined by summing the square of its value at every node and taking
 * the square root.
 */
class KokkosNodalL2Norm : public KokkosNodalSum
{
public:
  static InputParameters validParams();

  KokkosNodalL2Norm(const InputParameters & parameters);

  virtual Real getValue() const override;

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, Datum & datum) const
  {
    Real u = _u(datum, qp);
    return u * u;
  }
};
