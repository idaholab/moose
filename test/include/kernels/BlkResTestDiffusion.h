//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BLKRESTESTDIFFUSION_H
#define BLKRESTESTDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class BlkResTestDiffusion;

template <>
InputParameters validParams<BlkResTestDiffusion>();

InputParameters & modifyParams(InputParameters & params);

class BlkResTestDiffusion : public Kernel
{
public:
  BlkResTestDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // BLKRESTESTDIFFUSION_H
