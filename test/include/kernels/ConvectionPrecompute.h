//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelValue.h"

class ConvectionPrecompute : public KernelValue
{
public:
  static InputParameters validParams();

  ConvectionPrecompute(const InputParameters & parameters);

protected:
  Real precomputeQpResidual() override;
  Real precomputeQpJacobian() override;

private:
  RealVectorValue _velocity;
};
