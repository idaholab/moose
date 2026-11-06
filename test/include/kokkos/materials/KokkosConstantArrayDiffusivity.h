//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Declares a constant one-dimensional (diagonal) or two-dimensional (full matrix) diffusivity of
 * an array variable
 */
class KokkosConstantArrayDiffusivity : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosConstantArrayDiffusivity(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

protected:
  /// Number of array variable components
  const unsigned int _n;
  /// Whether the diffusivity is a two-dimensional property
  const bool _matrix;
  /// The diffusivity values, with the matrix entries in row-major order
  Moose::Kokkos::Array<Real> _values;
  /// The one-dimensional diffusivity
  Moose::Kokkos::MaterialProperty<Real, 1> _diffusivity_1d;
  /// The two-dimensional diffusivity
  Moose::Kokkos::MaterialProperty<Real, 2> _diffusivity_2d;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosConstantArrayDiffusivity::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  if (_matrix)
  {
    auto diffusivity = _diffusivity_2d(datum, qp);

    for (unsigned int i = 0; i < _n; ++i)
      for (unsigned int j = 0; j < _n; ++j)
        diffusivity(i, j) = _values[i * _n + j];
  }
  else
  {
    auto diffusivity = _diffusivity_1d(datum, qp);

    for (unsigned int i = 0; i < _n; ++i)
      diffusivity(i) = _values[i];
  }
}
