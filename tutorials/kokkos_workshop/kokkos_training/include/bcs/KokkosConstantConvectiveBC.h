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
 * Convective flux with a constant, controllable heat transfer coefficient
 */
class KokkosConstantConvectiveBC final : public KokkosConvectiveBCBase
{
public:
  static InputParameters validParams();

  KokkosConstantConvectiveBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeCoefficient(const unsigned int /* qp */,
                                          AssemblyDatum & /* datum */) const
  {
    return _h;
  }

private:
  /// Heat transfer coefficient, refreshed from the controllable parameter at every launch
  const Moose::Kokkos::Scalar<const Real> _h;
};
