//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REQUIREDPARAMETERSKERNEL_H
#define REQUIREDPARAMETERSKERNEL_H

#include "CoefDiffusion.h"

// Forward Declarations
class RequiredParametersKernel;

template <>
InputParameters validParams<RequiredParametersKernel>();

class RequiredParametersKernel : public CoefDiffusion
{
public:
  RequiredParametersKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // REQUIREDPARAMETERSKERNEL_H
