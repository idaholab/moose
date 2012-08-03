#include "CoupledKernelValueTest.h"


template<>
InputParameters validParams<CoupledKernelValueTest>()
{
  InputParameters p = validParams<KernelValue>();
  p.addRequiredCoupledVar("var2","Coupled Variable");
  return p;
}


CoupledKernelValueTest::CoupledKernelValueTest(const std::string & name, InputParameters parameters) :
    KernelValue(name, parameters),
    _var2(coupledValue("var2")),
    _var2_num(coupled("var2"))
{
}

CoupledKernelValueTest::~CoupledKernelValueTest()
{
}

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
    return _phi[_j][_qp]*_test[_i][_qp];
  }
  else
  {
    return 0;
  }
}
