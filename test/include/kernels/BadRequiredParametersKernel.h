//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BADREQUIREDPARAMETERSKERNEL_H
#define BADREQUIREDPARAMETERSKERNEL_H

#include "Kernel.h"

// Forward Declarations
class BadRequiredParametersKernel;

template <>
InputParameters validParams<BadRequiredParametersKernel>();

class BadRequiredParametersKernel : public Kernel
{
public:
  BadRequiredParametersKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // BADREQUIREDPARAMETERSKERNEL_H
