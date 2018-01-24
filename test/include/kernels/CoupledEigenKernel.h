//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDEIGENKERNEL_H
#define COUPLEDEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class CoupledEigenKernel;

template <>
InputParameters validParams<CoupledEigenKernel>();

class CoupledEigenKernel : public EigenKernel
{
public:
  CoupledEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const VariableValue & _v;
};

#endif // COUPLEDEIGENKERNEL_H
