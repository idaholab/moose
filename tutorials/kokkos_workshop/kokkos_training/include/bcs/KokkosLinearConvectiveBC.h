//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosConvectiveBCBase.h"

/**
 * Convective flux with the solution-dependent coefficient h(u) = h0 (1 + gamma u)
 */
class KokkosLinearConvectiveBC final : public KokkosConvectiveBCBase
{
public:
  static InputParameters validParams();

  KokkosLinearConvectiveBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeCoefficient(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _h0 * (1 + _gamma * _u(datum, qp));
  }
  KOKKOS_FUNCTION Real computeCoefficientDerivative(const unsigned int /* qp */,
                                                    AssemblyDatum & /* datum */) const
  {
    return _h0 * _gamma;
  }

private:
  /// Heat transfer coefficient at u = 0
  const Real _h0;
  /// Linear coefficient of the heat transfer coefficient in u
  const Real _gamma;
};
