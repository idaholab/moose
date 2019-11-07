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
 * This kernel implements (v, |u|^(p-2) u)/k, where u is the variable, v is the test function
 * and k is the eigenvalue. When p=2, this kernel is equivalent with MassEigenKernel.
 */
class PMassKernel : public Kernel
{
public:
  static InputParameters validParams();

  PMassKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Real _p;
  /// input parameter multiplied by the  kernel
  Real _coef;
};
