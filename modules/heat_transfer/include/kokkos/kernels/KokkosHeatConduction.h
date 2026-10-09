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
#include "KokkosDeformation.h"

/**
 * Kokkos kernel for a heat conduction term with an isotropic conductivity. When displacements are
 * coupled, the term is posed on the current configuration and pulled back to the reference
 * configuration:
 *
 *   (k grad_x T, grad_x psi)_{Omega_t} = (k J F^{-1} F^{-T} grad_X T, grad_X psi)_{Omega_0}
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
  KOKKOS_FUNCTION Real3 precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                                        const unsigned int jvar,
                                                        const unsigned int qp,
                                                        AssemblyDatum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _thermal_conductivity;
  Moose::Kokkos::MaterialProperty<Real> _d_thermal_conductivity_dT;

  /// Kinematics of the current configuration when the conduction is posed on it
  const Moose::Kokkos::Deformation _deformation;
};

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHeatConduction::precomputeQpResidualGrad(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real3 flux = _thermal_conductivity(datum, qp) * _grad_u(datum, qp);

  if (!_deformation)
    return flux;

  const auto kinematics = _deformation(datum, qp);
  return kinematics.J * (kinematics.F_inv * kinematics.grad(flux));
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

  if (!_deformation)
    return jac;

  const auto kinematics = _deformation(datum, qp);
  return kinematics.J * (kinematics.F_inv * kinematics.grad(jac));
}

template <typename Derived>
KOKKOS_FUNCTION Moose::Kokkos::Real3
KokkosHeatConduction::precomputeQpOffDiagJacobianGrad(const unsigned int j,
                                                      const unsigned int /* jvar */,
                                                      const unsigned int qp,
                                                      AssemblyDatum & datum) const
{
  const auto c = _deformation.component(datum);

  if (c == libMesh::invalid_uint)
    return Real3(0);

  // Differentiate J F^{-1} q with respect to displacement component c expanded in phi_j, where
  // q = k grad_x T. With h = grad_x phi_j, the derivatives of F^{-1} and F^{-T} are
  // -F^{-1} e_c (h . q) and -h q_c, respectively.
  const auto kinematics = _deformation(datum, qp);
  const auto q = kinematics.grad(_thermal_conductivity(datum, qp) * _grad_u(datum, qp));
  const auto h = kinematics.grad(_grad_phi(datum, j, qp));

  Real3 e_c(0);
  e_c(c) = 1;

  return kinematics.F_inv * (kinematics.dJ(c, _phi(datum, j, qp), _grad_phi(datum, j, qp)) * q -
                             kinematics.J * ((h * q) * e_c + q(c) * h));
}
