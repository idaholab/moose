//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "Material.h"

/**
 * Test material for checking selective material reinitialization, on boundaries and blocks.
 */
template <bool is_ad>
class MaterialReinitTestTempl : public Material
{
public:
  static InputParameters validParams();

  MaterialReinitTestTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Property supplied by this material
  GenericMaterialProperty<Real, is_ad> & _property;

  /// Constant contribution to the supplied property
  const Real _value;

  /// Optional material property dependency, retrieved with getGenericOrZeroMaterialProperty if
  /// or_zero_coupled_property is set
  const GenericMaterialProperty<Real, is_ad> * const _coupled_property;

  /// Whether computing the automatically-created face material is an error for this test
  const bool _error_on_face;
  /// Whether computing this material with boundary material data is an error for this test
  const bool _error_on_boundary;
  /// Whether computing the automatically-created neighbor material is an error for this test
  const bool _error_on_neighbor;
  /// Whether computing the block copy of this material is an error for this test
  const bool _error_on_volume;
};

typedef MaterialReinitTestTempl<false> MaterialReinitTest;
typedef MaterialReinitTestTempl<true> ADMaterialReinitTest;
