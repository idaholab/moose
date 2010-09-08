#include "PressureNeumannBC.h"

template<>
InputParameters validParams<PressureNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("component");
  return params;
}

PressureNeumannBC::PressureNeumannBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, parameters),
    _p(coupledValue("p")),
    _pe(coupledValue("pe")),
    _pu(coupledValue("pu")),
    _pv(coupledValue("pv")),
    _pw(_dim == 3 ? coupledValue("pw") : _zero),
    _component(getParam<Real>("component")),
    _gamma(getMaterialProperty<Real>("gamma"))
{
  if(_component < 0)
  {
    std::cout<<"Must select a component for PressureNeumannBC"<<std::endl;
    libmesh_error();
  }
}

Real
PressureNeumannBC::pressure()
{

  Real _u_vel = _pu[_qp] / _p[_qp];
  Real _v_vel = _pv[_qp] / _p[_qp];
  Real _w_vel = _pw[_qp] / _p[_qp];

  return (_gamma[_qp] - 1)*(_pe[_qp] - (0.5 * (_u_vel*_u_vel + _v_vel*_v_vel + _w_vel*_w_vel)));
}

Real
PressureNeumannBC::computeQpResidual()
{
  return pressure()*_normals[_qp](_component)*_phi[_i][_qp];
}
