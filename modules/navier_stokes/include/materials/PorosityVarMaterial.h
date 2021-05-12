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

class Function;

class PorosityVarMaterial : public Material
{
public:
  PorosityVarMaterial(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual void computeQpProperties() override;

  const Function * const _eps_function;
  const ADVariableValue * const _eps_var_value;
  const ADVariableGradient * const _eps_var_grad;
  MaterialProperty<Real> & _eps;
  MaterialProperty<RealVectorValue> & _eps_grad;
};
