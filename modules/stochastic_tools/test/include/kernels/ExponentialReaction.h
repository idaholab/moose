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

class ExponentialReaction : public Kernel
{
public:
  static InputParameters validParams();

  ExponentialReaction(const InputParameters & parameters);

protected:

  const Real & _mu1;

  const Real & _mu2;

  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};
