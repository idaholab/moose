//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernel.h"

/**
 * Represents a right hand side force term of the form
 * Sum_j c_j * m_j * v_j, where c is a vector of real numbers,
 * m_j is a vector of material properties, and v_j is a vector
 * of variables
 */
class KokkosMatCoupledForce final : public Moose::Kokkos::Kernel<KokkosMatCoupledForce>
{
public:
  static InputParameters validParams();

  KokkosMatCoupledForce(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;
  KOKKOS_FUNCTION inline Real computeQpOffDiagJacobian(const unsigned int i,
                                                       const unsigned int j,
                                                       const unsigned int jvar,
                                                       const unsigned int qp,
                                                       ResidualDatum & datum) const;

private:
  const unsigned int _n_coupled;
  const bool _coupled_props;
  std::vector<unsigned int> _v_var;
  const Moose::Kokkos::VariableValue _v;
  Moose::Kokkos::Array<Real> _coef;
  Moose::Kokkos::Map<unsigned int, unsigned int> _v_var_to_index;
  Moose::Kokkos::Array<Moose::Kokkos::MaterialProperty<Real>> _mat_props;
};

KOKKOS_FUNCTION inline Real
KokkosMatCoupledForce::computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
{
  Real r = 0;
  if (_coupled_props)
    for (unsigned int j = 0; j < _n_coupled; ++j)
      r += -_coef[j] * _mat_props[j](datum, qp) * _v(datum, qp, j);
  else
    for (unsigned int j = 0; j < _n_coupled; ++j)
      r += -_coef[j] * _v(datum, qp, j);
  return r * _test(datum, i, qp);
}

KOKKOS_FUNCTION inline Real
KokkosMatCoupledForce::computeQpOffDiagJacobian(const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int qp,
                                                ResidualDatum & datum) const
{
  auto idx = _v_var_to_index.find(jvar);
  if (idx == _v_var_to_index.size())
    return 0;

  unsigned int p = _v_var_to_index[idx];

  if (_coupled_props)
    return -_coef[p] * _mat_props[p](datum, qp) * _phi(datum, j, qp) * _test(datum, i, qp);
  return -_coef[p] * _phi(datum, j, qp) * _test(datum, i, qp);
}
