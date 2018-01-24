//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledKernelValueTest.h"

template <>
InputParameters
validParams<CoupledKernelValueTest>()
{
  InputParameters p = validParams<KernelValue>();
  p.addRequiredCoupledVar("var2", "Coupled Variable");
  return p;
}

CoupledKernelValueTest::CoupledKernelValueTest(const InputParameters & parameters)
  : KernelValue(parameters), _var2(coupledValue("var2")), _var2_num(coupled("var2"))
{
}

CoupledKernelValueTest::~CoupledKernelValueTest() {}

Real
CoupledKernelValueTest::precomputeQpResidual()
{
  return _var2[_qp];
}

Real
CoupledKernelValueTest::precomputeQpJacobian()
{
  return 0;
}

Real
CoupledKernelValueTest::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var2_num)
  {
    return _phi[_j][_qp] * _test[_i][_qp];
  }
  else
  {
    return 0;
  }
}
