//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDKERNELVALUETEST_H
#define COUPLEDKERNELVALUETEST_H

#include "KernelValue.h"

class CoupledKernelValueTest;

template <>
InputParameters validParams<CoupledKernelValueTest>();

class CoupledKernelValueTest : public KernelValue
{
public:
  CoupledKernelValueTest(const InputParameters & parameters);
  virtual ~CoupledKernelValueTest();

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  const VariableValue & _var2;
  unsigned int _var2_num;
};

#endif /* COUPLEDKERNELVALUETEST_H */
