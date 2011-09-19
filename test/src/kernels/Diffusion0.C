#include "Diffusion0.h"

template<>
InputParameters validParams<Diffusion0>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("Ak")= 1.;
  params.set<Real>("Bk")= 1.;
  params.set<Real>("Ck")= 1.;

  return params;
}

Diffusion0::Diffusion0(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters)
{
  _Ak = getParam<Real>("Ak");
  _Bk = getParam<Real>("Bk");
  _Ck = getParam<Real>("Ck");
}

Real
Diffusion0::computeQpResidual()
{
  Real diff = _Ak+_Bk*_q_point[_qp](0)+_Ck*_q_point[_qp](1);

  return diff*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
Diffusion0::computeQpJacobian()
{
  Real diff = _Ak+_Bk*_q_point[_qp](0)+_Ck*_q_point[_qp](1);

  return diff*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
