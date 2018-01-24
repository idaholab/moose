//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RESTARTDIFFUSION_H
#define RESTARTDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class RestartDiffusion;

template <>
InputParameters validParams<RestartDiffusion>();

class RestartDiffusion : public Kernel
{
public:
  RestartDiffusion(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
  Real & _current_coef;
};

#endif // RESTARTDIFFUSION_H
