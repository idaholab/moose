//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * A material that optinally computes two properties, one corresponding to the value of the coupled
 * variable and the other corresponding to the gradient of the coupled variable
 */
template <bool is_ad>
class CoupledGradientMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  CoupledGradientMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Material property computed, equal to the gradient of the variable times a scalar
  GenericMaterialProperty<RealVectorValue, is_ad> & _grad_mat_prop;
  /// A scalar material property that acts as a factor in the computed property
  const GenericMaterialProperty<Real, is_ad> & _scalar_property_factor;
  /// Gradient of the variable
  const GenericVariableGradient<is_ad> & _grad_u;
};

typedef CoupledGradientMaterialTempl<false> CoupledGradientMaterial;
typedef CoupledGradientMaterialTempl<true> ADCoupledGradientMaterial;
