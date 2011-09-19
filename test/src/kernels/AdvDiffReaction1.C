#include "AdvDiffReaction1.h"

template<>
InputParameters validParams<AdvDiffReaction1>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("A0")=0.;
  params.set<Real>("B0")=0.;
  params.set<Real>("C0")=0.;

  params.set<Real>("Au")=1.;
  params.set<Real>("Bu")=1.;
  params.set<Real>("Cu")=1.;

  params.set<Real>("Av")=1.;
  params.set<Real>("Bv")=1.;
  params.set<Real>("Cv")=1.;

  params.set<Real>("Ak")=1.;
  params.set<Real>("Bk")=1.;
  params.set<Real>("Ck")=1.;

  params.set<Real>("omega0")=1.;

  return params;
}

AdvDiffReaction1::AdvDiffReaction1(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters)
{
  _A0 = getParam<Real>("A0");
  _B0 = getParam<Real>("B0");
  _C0 = getParam<Real>("C0");

  _Au = getParam<Real>("Au");
  _Bu = getParam<Real>("Bu");
  _Cu = getParam<Real>("Cu");

  _Av = getParam<Real>("Av");
  _Bv = getParam<Real>("Bv");
  _Cv = getParam<Real>("Cv");

  _Ak = getParam<Real>("Ak");
  _Bk = getParam<Real>("Bk");
  _Ck = getParam<Real>("Ck");

  _omega0 = getParam<Real>("omega0");
}

Real
AdvDiffReaction1::computeQpResidual()
{
  // Reaction:
  Real QpResidual = _test[_i][_qp]*ManSol4ADR2src(_q_point[_qp],
                                                  _A0,_B0,_C0,_Au,_Bu,_Cu,_Av,_Bv,_Cv,_Ak,_Bk,_Ck,
                                                  _omega0,_t);
  // Advection:
  VectorValue<Number> vel(_Au+_Bu*_q_point[_qp](0)+_Cu*_q_point[_qp](1),
                          _Av+_Bv*_q_point[_qp](0)+_Cv*_q_point[_qp](1),0);
  QpResidual += -_test[_i][_qp]*vel*_grad_u[_qp];

  // Diffusion:
  Real diff   = _Ak+_Bk*_q_point[_qp](0)+_Ck*_q_point[_qp](1);
  QpResidual += diff*_grad_test[_i][_qp]*_grad_u[_qp];

  return QpResidual;

}

Real
AdvDiffReaction1::computeQpJacobian()
{
  // Advection:
  VectorValue<Number> vel(_Au+_Bu*_q_point[_qp](0)+_Cu*_q_point[_qp](1),
                          _Av+_Bv*_q_point[_qp](0)+_Cv*_q_point[_qp](1),0);
  Real Jacob = -_test[_i][_qp]*vel*_grad_phi[_j][_qp];

  // Diffusion:
  Real diff = _Ak+_Bk*_q_point[_qp](0)+_Ck*_q_point[_qp](1);
  Jacob    += diff*_grad_test[_i][_qp]*_grad_phi[_j][_qp];

  return Jacob;
}
