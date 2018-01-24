/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
