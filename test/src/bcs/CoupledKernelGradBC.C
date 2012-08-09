#include "CoupledKernelGradBC.h"
#include "Function.h"

template<>
InputParameters validParams<CoupledKernelGradBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("var2","Coupled Variable");
  params.addRequiredParam<std::vector<Real> >("vel","velocity");
  return params;
}

CoupledKernelGradBC::CoupledKernelGradBC(const std::string & name, InputParameters parameters) :
  IntegratedBC(name, parameters),
  _var2(coupledValue("var2"))
{
  std::vector<Real> a(getParam<std::vector<Real> >("vel"));
  if (a.size()!=2)
  {
    mooseError("ERROR: CoupledKernelGradBC only implemented for 2d, vel is not size 2");
  }
  _beta=RealVectorValue(a[0],a[1]);
}

CoupledKernelGradBC::~CoupledKernelGradBC()
{}

Real
CoupledKernelGradBC::computeQpResidual()
{
  return _test[_i][_qp]*((_beta*_var2[_qp])*_normals[_qp]);
}
