//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"

/**
 * Mass flow rate from the matrix to the porespace.  Add this to TimeDerivative kernel to get
 * complete DE for the fluid adsorbed in the matrix
 */
class KokkosDesorptionFromMatrix : public Moose::Kokkos::KernelValue
{
public:
  static InputParameters validParams();

  KokkosDesorptionFromMatrix(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _mass_rate_from_matrix(datum, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const
  {
    return _dmass_rate_from_matrix_dC(datum, qp) * _phi(datum, j, qp);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpOffDiagJacobian(const unsigned int j,
                                                   const unsigned int jvar,
                                                   const unsigned int qp,
                                                   AssemblyDatum & datum) const
  {
    return jvar == _coupled_var ? _dmass_rate_from_matrix_dp(datum, qp) * _phi(datum, j, qp) : 0;
  }

private:
  /// Variable number of the coupled variable
  const unsigned int _coupled_var;
  /// Mass flow rate from matrix = mass flow rate to porespace
  const Moose::Kokkos::MaterialProperty<Real> _mass_rate_from_matrix;
  /// Derivative of mass flow rate from matrix wrt concentration
  const Moose::Kokkos::MaterialProperty<Real> _dmass_rate_from_matrix_dC;
  /// Derivative of mass flow rate from matrix wrt pressure
  const Moose::Kokkos::MaterialProperty<Real> _dmass_rate_from_matrix_dp;
};
