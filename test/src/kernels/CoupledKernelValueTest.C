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
