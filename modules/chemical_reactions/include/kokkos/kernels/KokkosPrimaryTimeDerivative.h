//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeDerivative.h"

/**
 * Kokkos time derivative of the primary species concentration scaled by the porosity
 */
class KokkosPrimaryTimeDerivative : public KokkosTimeDerivative
{
public:
  static InputParameters validParams();

  KokkosPrimaryTimeDerivative(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Porosity
  const Moose::Kokkos::MaterialProperty<Real> _porosity;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPrimaryTimeDerivative::precomputeQpResidual(const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  return _porosity(datum, qp) * KokkosTimeDerivative::precomputeQpResidual<Derived>(qp, datum);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPrimaryTimeDerivative::precomputeQpJacobian(const unsigned int j,
                                                  const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  return _porosity(datum, qp) * KokkosTimeDerivative::precomputeQpJacobian<Derived>(j, qp, datum);
}
