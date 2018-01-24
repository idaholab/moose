//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MASSEIGENKERNEL_H
#define MASSEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class MassEigenKernel;

template <>
InputParameters validParams<MassEigenKernel>();

class MassEigenKernel : public EigenKernel
{
public:
  MassEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};

#endif // MASSEIGENKERNEL_H
