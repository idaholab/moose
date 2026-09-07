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
 * Test material for checking selective material reinitialization on boundaries.
 */
class BoundaryMaterialReinitTest : public Material
{
public:
  static InputParameters validParams();

  BoundaryMaterialReinitTest(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Property supplied by this material
  MaterialProperty<Real> & _property;

  /// Constant contribution to the supplied property
  const Real _value;

  /// Optional material property dependency
  const MaterialProperty<Real> * const _coupled_property;

  /// Whether computing the automatically-created face material is an error for this test
  const bool _error_on_face;
};
