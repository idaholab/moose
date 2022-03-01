//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

class Function;

/**
 * Represents the rate in a simple ODE of du/dt = f
 */
class UserForcingFunctionNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor grabs the Function
   */
  static InputParameters validParams();

  UserForcingFunctionNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  const Function & _func;
};
