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
