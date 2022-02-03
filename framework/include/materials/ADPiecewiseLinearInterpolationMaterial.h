//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "LinearInterpolation.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * This material uses a LinearInterpolation object to define the dependence
 * of the material's value on a variable.
 */
class ADPiecewiseLinearInterpolationMaterial : public Material,
                                               public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  ADPiecewiseLinearInterpolationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Name of the property to be computed
  const std::string _prop_name;

  /// Value of the coupled variable to be used as the abscissa in the piecewise linear interpolation
  const ADVariableValue & _coupled_var;

  /// Factor to scale the ordinate values by (default = 1)
  const Real _scale_factor;

  /// use linear extrapolation
  const bool _extrap;

  /// Material property to be calculated
  ADMaterialProperty<Real> & _property;

  /// LinearInterpolation object
  std::unique_ptr<LinearInterpolation> _linear_interp;
};
