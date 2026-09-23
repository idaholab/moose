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
 * Small strain tensor computed from a single vector displacement variable.
 */
class KokkosComputeSmallStrain : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosComputeSmallStrain(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// Base name prefixing the declared property names
  const std::string _base_name;

  /// Gradient of the vector displacement variable, indexed as (component, spatial direction)
  const Moose::Kokkos::VectorVariableGradient _grad_disp;

  /// Total strain
  Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _total_strain;
  /// Mechanical strain, which equals the total strain in the absence of eigenstrains
  Moose::Kokkos::MaterialProperty<Moose::Kokkos::Real33> _mechanical_strain;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosComputeSmallStrain::computeQpProperties(const unsigned int qp, Datum & datum) const
{
  const auto grad = _grad_disp(datum, qp);

  // strain = (grad_disp + grad_disp^T) / 2
  Moose::Kokkos::Real33 strain = grad;
  strain += grad.transpose();
  strain *= 0.5;

  _total_strain(datum, qp) = strain;
  _mechanical_strain(datum, qp) = strain;
}
