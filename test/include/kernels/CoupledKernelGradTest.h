//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDKERNELGRADTEST_H
#define COUPLEDKERNELGRADTEST_H

#include "KernelGrad.h"

class CoupledKernelGradTest;

template <>
InputParameters validParams<CoupledKernelGradTest>();

class CoupledKernelGradTest : public KernelGrad
{
public:
  CoupledKernelGradTest(const InputParameters & parameters);
  virtual ~CoupledKernelGradTest();

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  RealVectorValue _beta;
  const VariableValue & _var2;
  unsigned int _var2_num;
};

#endif /* COUPLEDKERNELGRADTEST_H */
