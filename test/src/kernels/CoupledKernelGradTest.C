#include "CoupledKernelGradTest.h"


template<>
InputParameters validParams<CoupledKernelGradTest>()
{
  InputParameters p = validParams<KernelGrad>();
  p.addRequiredCoupledVar("var2","Coupled Variable");
  p.addRequiredParam<std::vector<Real> >("vel","velocity");
  return p;
}


CoupledKernelGradTest::CoupledKernelGradTest(const std::string & name, InputParameters parameters) :
    KernelGrad(name, parameters),
    _var2(coupledValue("var2")),
    _var2_num(coupled("var2"))
{
  std::vector<Real> a(getParam<std::vector<Real> >("vel"));
  if (a.size()!=2)
  {
    mooseError("ERROR: CoupledKernelGradTest only implemented for 2D, vel is not size 2");
  }
  _beta=RealVectorValue(a[0],a[1]);
}

CoupledKernelGradTest::~CoupledKernelGradTest()
{
}

RealGradient
CoupledKernelGradTest::precomputeQpResidual()
{
  return -_beta*_var2[_qp];
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
    //return _beta*_var2[_qp]*_phi[_j][_qp]*_grad_test[_i][_qp];
    return -(_beta*_phi[_j][_qp])*_grad_test[_i][_qp];
  }
  else
  {
    return 0;
  }
}
