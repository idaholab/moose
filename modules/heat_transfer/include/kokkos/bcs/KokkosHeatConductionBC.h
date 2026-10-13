//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"

/**
 * Kokkos boundary condition for a diffusive heat flux \f$ -k \nabla T \cdot \hat{n} \f$
 */
class KokkosHeatConductionBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosHeatConductionBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Thermal conductivity
  const Moose::Kokkos::MaterialProperty<Real> _k;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatConductionBC::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return _k(datum, qp) * (_grad_u(datum, qp) * datum.normals(qp));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosHeatConductionBC::precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
{
  return _k(datum, qp) * (_grad_phi(datum, j, qp) * datum.normals(qp));
}
