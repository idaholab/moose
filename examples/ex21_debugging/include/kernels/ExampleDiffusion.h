//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Diffusion.h"

class ExampleDiffusion : public Diffusion
{
public:
  ExampleDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /**
   * THIS IS AN ERROR ON PURPOSE!
   *
   * The "&" is missing here!
   *
   * Do NOT copy this line of code!
   */

  const VariableValue _coupled_coef;
};
