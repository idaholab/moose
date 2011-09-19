#include "Advection0.h"


template<>
InputParameters validParams<Advection0>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("Au")= 1.;
  params.set<Real>("Bu")= 1.;
  params.set<Real>("Cu")= 1.;

  params.set<Real>("Av")= 1.;
  params.set<Real>("Bv")= 1.;
  params.set<Real>("Cv")= 1.;

  return params;
}

Advection0::Advection0(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters)
{
  _Au = getParam<Real>("Au");
  _Bu = getParam<Real>("Bu");
  _Cu = getParam<Real>("Cu");

  _Av = getParam<Real>("Av");
  _Bv = getParam<Real>("Bv");
  _Cv = getParam<Real>("Cv");
}

Real
Advection0::computeQpResidual()
{
  VectorValue<Number> vel(_Au+_Bu*_q_point[_qp](0)+_Cu*_q_point[_qp](1),
                          _Av+_Bv*_q_point[_qp](0)+_Cv*_q_point[_qp](1),0);
  return -_test[_i][_qp]*vel*_grad_u[_qp];
}

Real
Advection0::computeQpJacobian()
{
  VectorValue<Number> vel(_Au+_Bu*_q_point[_qp](0)+_Cu*_q_point[_qp](1),
                          _Av+_Bv*_q_point[_qp](0)+_Cv*_q_point[_qp](1),0);
  return -_test[_i][_qp]*vel*_grad_phi[_j][_qp];
}
