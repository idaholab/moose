//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementIntegralPostprocessor.h"

/**
 * Kokkos postprocessor computing an entry of the homogenized thermal conductivity tensor by
 * asymptotic expansion homogenization
 */
class KokkosHomogenizedThermalConductivity : public KokkosElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  KokkosHomogenizedThermalConductivity(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const;

private:
  /// Row index of the homogenized thermal conductivity tensor entry
  const unsigned int _row;
  /// Column index of the homogenized thermal conductivity tensor entry
  const unsigned int _col;
  /// Scale factor
  const Real _scale;
  /// Mesh dimension
  const unsigned int _dim;
  /// Gradients of the characteristic functions
  const Moose::Kokkos::VariableGradient _grad_chi;
  /// Whether the diffusion coefficient is a tensor
  const bool _is_tensor;
  /// Scalar diffusion coefficient
  const Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient;
  /// Tensor diffusion coefficient
  const Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _tensor_diffusion_coefficient;
};

KOKKOS_FUNCTION inline Real
KokkosHomogenizedThermalConductivity::computeQpIntegral(const unsigned int qp, Datum & datum) const
{
  // the _row-th row of the thermal conductivity tensor
  Moose::Kokkos::Real3 k_row(0);
  if (!_is_tensor)
    k_row(_row) = _diffusion_coefficient(datum, qp);
  else
  {
    const Moose::Kokkos::Real33 k = _tensor_diffusion_coefficient(datum, qp);
    for (unsigned int j = 0; j < _dim; ++j)
      k_row(j) = k(_row, j);
  }

  // initialize the dchi/dx tensor, but we only use _col-th column
  Moose::Kokkos::Real3 M_col(0);
  M_col(_col) = 1.0;
  const auto grad_chi = _grad_chi(datum, qp, _col);
  for (unsigned int i = 0; i < _dim; ++i)
    M_col(i) += grad_chi(i);

  return _scale * (k_row * M_col);
}
