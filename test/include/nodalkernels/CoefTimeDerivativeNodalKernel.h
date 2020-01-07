//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivativeNodalKernel.h"

class CoefTimeDerivativeNodalKernel : public TimeDerivativeNodalKernel
{
public:
  static InputParameters validParams();

  CoefTimeDerivativeNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Real _coeff;
};
