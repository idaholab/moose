//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * A test class for checking the operation for BlockRestrictable::hasMaterialProperty
 */
class MatCoefDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  MatCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  std::string _prop_name;
  const MaterialProperty<Real> * _coef;
};
