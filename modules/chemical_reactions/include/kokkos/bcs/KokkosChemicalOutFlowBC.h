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
 * Kokkos chemical flux boundary condition
 */
class KokkosChemicalOutFlowBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosChemicalOutFlowBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Diffusivity
  const Moose::Kokkos::MaterialProperty<Real> _diff;
  /// Porosity
  const Moose::Kokkos::MaterialProperty<Real> _porosity;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosChemicalOutFlowBC::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  return -_porosity(datum, qp) * _diff(datum, qp) * (_grad_u(datum, qp) * datum.normals(qp));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosChemicalOutFlowBC::precomputeQpJacobian(const unsigned int j,
                                              const unsigned int qp,
                                              AssemblyDatum & datum) const
{
  return -_porosity(datum, qp) * _diff(datum, qp) * (_grad_phi(datum, j, qp) * datum.normals(qp));
}
