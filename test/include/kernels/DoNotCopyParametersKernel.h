//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DONOTCOPYPARAMETESKERNEL_H
#define DONOTCOPYPARAMETESKERNEL_H

#include "Kernel.h"

// Forward Declarations
class DoNotCopyParametersKernel;

template <>
InputParameters validParams<DoNotCopyParametersKernel>();

class DoNotCopyParametersKernel : public Kernel
{
public:
  // This is the wrong constructor, don't to this!
  DoNotCopyParametersKernel(InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // DONOTCOPYPARAMETESKERNEL_H
