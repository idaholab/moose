//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementIntegralVariablePostprocessor.h"

class KokkosElementL2Norm : public KokkosElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  KokkosElementL2Norm(const InputParameters & parameters);

  virtual Real getValue() const override;

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const
  {
    Real u = _u(datum, qp);
    return u * u;
  }
};
