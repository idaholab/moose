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

/**
 * A material that computes two properties, one corresponding to the value of the coupled variable
 * and the other corresponding to the gradient of the coupled variable
 */
class ADCoupledGradientMaterial : public Material
{
public:
  static InputParameters validParams();

  ADCoupledGradientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const std::string _mat_prop_name;
  const std::string _grad_mat_prop_name;
  ADMaterialProperty<Real> & _mat_prop;
  ADMaterialProperty<RealVectorValue> & _grad_mat_prop;
  const ADVariableValue & _u;
  const ADVariableGradient & _grad_u;
};
