#include "CoupledEigenKernel.h"

template<>
InputParameters validParams<CoupledEigenKernel>()
{
  InputParameters params = validParams<EigenKernel>();
  params.addRequiredCoupledVar("v", "Variable to be coupled in");
  return params;
}

CoupledEigenKernel::CoupledEigenKernel(const std::string & name, InputParameters parameters) :
    EigenKernel(name,parameters),
    _v(coupledValue("v"))
{
}

Real
CoupledEigenKernel::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}
