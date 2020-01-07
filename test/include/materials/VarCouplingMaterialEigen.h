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
 * A material that couples a variable
 */
class VarCouplingMaterialEigen : public Material
{
public:
  static InputParameters validParams();

  VarCouplingMaterialEigen(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const VariableValue & _var;
  const VariableValue & _var_old;
  std::string _propname;
  MaterialProperty<Real> & _mat;
  MaterialProperty<Real> & _mat_old;
};
