//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class RestartDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  RestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
  Real & _current_coef;
  int & _last_t_step;
};
