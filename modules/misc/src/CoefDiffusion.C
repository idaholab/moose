#include "CoefDiffusion.h"


template<>
InputParameters validParams<CoefDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("coef")=0.0;
  return params;
}

CoefDiffusion::CoefDiffusion(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _coef(getParam<Real>("coef"))
{}

Real
CoefDiffusion::computeQpResidual()
{
  return _coef*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  return _coef*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
