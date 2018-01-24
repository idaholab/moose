//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DOTCOUPLINGKERNEL_H
#define DOTCOUPLINGKERNEL_H

#include "Kernel.h"

class DotCouplingKernel;

template <>
InputParameters validParams<DotCouplingKernel>();

/**
 * Kernel that is calling coupledDot
 */
class DotCouplingKernel : public Kernel
{
public:
  DotCouplingKernel(const InputParameters & parameters);
  virtual ~DotCouplingKernel(){};

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _v_dot;
  const VariableValue & _dv_dot_dv;
};

#endif /* DOTCOUPLINGKERNEL_H */
