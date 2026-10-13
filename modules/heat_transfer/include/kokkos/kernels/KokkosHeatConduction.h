//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelGrad.h"

/**
 * Kokkos kernel for a heat conduction term with an isotropic conductivity
 */
class KokkosHeatConduction : public Moose::Kokkos::KernelGrad
{
  using Real3 = Moose::Kokkos::Real3;

public:
  static InputParameters validParams();

  KokkosHeatConduction(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpResidualGrad(const unsigned int qp,
                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpJacobianGrad(const unsigned int j,
                                                 const unsigned int qp,
                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real3 precomputeQpPositionJacobianGrad(const Real3 & dx,
                                                         const unsigned int qp,
                                                         AssemblyDatum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _d_thermal_conductivity_dT;
  /// Gradient of the thermal conductivity with respect to the position, which carries the
  /// dependence of a conductivity evaluated on the current configuration on the displacements
  Moose::Kokkos::MaterialProperty<Real3> _d_thermal_conductivity_dx;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHeatConduction::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  return _thermal_conductivity(datum, qp) * _grad_u(datum, qp);
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHeatConduction::precomputeQpJacobianGrad(const unsigned int j,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const
{
  Real3 jac = _thermal_conductivity(datum, qp) * _grad_phi(datum, j, qp);
  if (_d_thermal_conductivity_dT)
    jac += _d_thermal_conductivity_dT(datum, qp) * _phi(datum, j, qp) * _grad_u(datum, qp);
  return jac;
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHeatConduction::precomputeQpPositionJacobianGrad(const Real3 & dx,
                                                       const unsigned int qp,
                                                       AssemblyDatum & datum) const
{
  if (!_d_thermal_conductivity_dx)
    return Real3(0);

  const Real3 dk_dx = _d_thermal_conductivity_dx(datum, qp);

  return (dk_dx * dx) * _grad_u(datum, qp);
}
