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

class ExampleMaterial : public Material
{
public:
  ExampleMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

private:
  /// member variable to hold the computed diffusivity coefficient
  MaterialProperty<Real> & _diffusivity;
  /// member variable to hold the computed convection velocity gradient term
  MaterialProperty<RealGradient> & _convection_velocity;

  /// A place to store the coupled variable gradient for calculating the convection velocity
  /// property.
  const VariableGradient & _diffusion_gradient;

  /// A helper object for performaing linear interpolations on tabulated data for calculating the
  /// diffusivity property.
  LinearInterpolation _piecewise_func;
};
