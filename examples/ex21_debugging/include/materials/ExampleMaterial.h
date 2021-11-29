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

/**
 * Example material class that defines a few properties.
 */
class ExampleMaterial : public Material
{
public:
  ExampleMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

private:
  /**
   * This is the member reference that will hold the computed values
   * for the Real value property in this class.
   */
  MaterialProperty<Real> & _diffusivity;

  /**
   * Computed values for the Gradient value property in this class.
   */
  MaterialProperty<RealGradient> & _convection_velocity;

  /**
   * This is the member reference that will hold the gradient
   * of the coupled variable
   */
  const VariableGradient & _diffusion_gradient;

  /**
   * This object returns a piecewise linear function based an a series
   * of points and their corresponding values
   */
  LinearInterpolation _piecewise_func;
};
