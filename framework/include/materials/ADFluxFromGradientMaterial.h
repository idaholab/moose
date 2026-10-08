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
 * Material computing a diffusive flux from the gradient of a coupled variable.
 *
 * J = -D * grad(u)
 */
class ADFluxFromGradientMaterial : public Material
{
public:
  static InputParameters validParams();

  ADFluxFromGradientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Gradient of the coupled variable
  const ADVariableGradient & _grad_u;
  /// Diffusivity
  const ADMaterialProperty<Real> & _diffusivity;
  /// Material property storing the flux vector
  ADMaterialProperty<RealVectorValue> & _flux;
};
