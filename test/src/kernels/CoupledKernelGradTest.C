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
#include "CoupledKernelGradTest.h"

template <>
InputParameters
validParams<CoupledKernelGradTest>()
{
  InputParameters p = validParams<KernelGrad>();
  p.addRequiredCoupledVar("var2", "Coupled Variable");
  p.addRequiredParam<std::vector<Real>>("vel", "velocity");
  p.addParam<bool>("test_coupling_error",
                   false,
                   "Set to true to verify that error messages are "
                   "produced if a coupling is requested that wasn't "
                   "declared");
  return p;
}

CoupledKernelGradTest::CoupledKernelGradTest(const InputParameters & parameters)
  : KernelGrad(parameters), _var2(coupledValue("var2")), _var2_num(coupled("var2"))
{
  std::vector<Real> a(getParam<std::vector<Real>>("vel"));
  if (a.size() != 2)
  {
    mooseError("ERROR: CoupledKernelGradTest only implemented for 2D, vel is not size 2");
  }
  _beta = RealVectorValue(a[0], a[1]);

  // Test coupling error
  if (getParam<bool>("test_coupling_error"))
    coupledGradient("var_undeclared");
}

CoupledKernelGradTest::~CoupledKernelGradTest() {}

RealGradient
CoupledKernelGradTest::precomputeQpResidual()
{
  return -_beta * _var2[_qp];
}

RealGradient
CoupledKernelGradTest::precomputeQpJacobian()
{
  return 0;
}

Real
CoupledKernelGradTest::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var2_num)
  {
    // return _beta*_var2[_qp]*_phi[_j][_qp]*_grad_test[_i][_qp];
    return -(_beta * _phi[_j][_qp]) * _grad_test[_i][_qp];
  }
  else
  {
    return 0;
  }
}
