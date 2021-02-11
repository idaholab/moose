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

class PressureEqualsDensityMaterial : public Material
{
public:
  static InputParameters validParams();

  PressureEqualsDensityMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  const ADVariableValue & _rho;
  const ADVariableGradient & _grad_rho;
  const ADVariableValue & _rho_u;
  const ADVariableValue & _rho_v;
  const ADVariableValue & _rho_w;
  const ADVariableValue & _rho_et;
  ADMaterialProperty<RealVectorValue> & _velocity;
  ADMaterialProperty<RealVectorValue> & _grad_p;
  ADMaterialProperty<Real> & _rho_ht;
};
