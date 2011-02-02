#include "CHSplit2LaPl.h"
// The couple, CHSplit2LaPl and SplitCHBlackLZ, splits the CH equation by replacing chemical potential with 'w'.
template<>
InputParameters validParams<CHSplit2LaPl>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("kappa_name","The kappa used with the kernel");  
  params.addRequiredCoupledVar("c","concentration");

  return params;
}

CHSplit2LaPl::CHSplit2LaPl(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _kappa_name(getParam<std::string>("kappa_name")),
   _kappa(getMaterialProperty<Real>(_kappa_name)),
   _c_var(coupled("c")),
   _grad_c(coupledGradient("c"))
{}

Real
CHSplit2LaPl::computeQpResidual()
{
  return -_kappa[_qp]*_grad_c[_qp] * _grad_test[_i][_qp];
}

Real
CHSplit2LaPl::computeQpJacobian()
{
  return 0;
}

Real
CHSplit2LaPl::computeQpOffDiagJacobian(unsigned int jvar)
{

  if(jvar == _c_var)
  {
    return -_kappa[_qp]*_grad_phi[_j][_qp] * _grad_test[_i][_qp];
  }

  return 0.0;
}

