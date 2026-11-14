//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTimeKernelValue.h"

/**
 * Kokkos derivative of the kinetic species concentration with respect to time, discretized with
 * the backward Euler method
 */
class KokkosCoupledBEKinetic : public Moose::Kokkos::TimeKernelValue
{
public:
  static InputParameters validParams();

  KokkosCoupledBEKinetic(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  /// The time derivative Jacobian of the kernel variable kept from the original object, although
  /// the residual does not depend on the kernel variable
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const
  {
    return _phi(datum, j, qp) * _du_dot_du;
  }

private:
  /// Porosity
  const Moose::Kokkos::MaterialProperty<Real> _porosity;
  /// Weights of the kinetic species concentrations
  Moose::Kokkos::Array<Real> _weight;
  /// Number of coupled kinetic species
  const unsigned int _n;
  /// Coupled kinetic species concentrations
  const Moose::Kokkos::VariableValue _vals;
  /// Old coupled kinetic species concentrations
  const Moose::Kokkos::VariableValue _vals_old;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosCoupledBEKinetic::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  Real assemble_conc = 0;

  for (unsigned int i = 0; i < _n; ++i)
    assemble_conc += _weight[i] * (_vals(datum, qp, i) - _vals_old(datum, qp, i)) / _dt;

  return _porosity(datum, qp) * assemble_conc;
}
