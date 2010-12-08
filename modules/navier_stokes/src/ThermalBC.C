#include "ThermalBC.h"

template<>
InputParameters validParams<ThermalBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("initial")=0.0;
  params.set<Real>("final")=0.0;
  params.set<Real>("duration")=0.0;

  params.set<bool>("_integrated") = false;
  return params;
}

ThermalBC::ThermalBC(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
    _p_var(coupled("p")),
    _p(coupledValue("p")),
    _u_vel_var(coupled("u")),
    _u_vel(coupledValue("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledValue("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : 0),
    _w_vel(_dim == 3 ? coupledValue("w") : _zero),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration")),
    _gamma(getMaterialProperty<Real>("gamma")),
    _R(getMaterialProperty<Real>("R")),
    _c_v(getMaterialProperty<Real>("c_v"))
  {}

Real
ThermalBC::temperature()
{
  Real value = 1.0/_c_v[_qp];

  Real et = _u[_qp]/_p[_qp];

  RealVectorValue vec(_u_vel[_qp],_v_vel[_qp],_w_vel[_qp]);

  value *= et - ((vec * vec) / 2.0);

  return value;
}
  
Real
ThermalBC::computeQpResidual()
{
  Real value;
  
  if(_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
  else
    value = _final;

  return temperature() - value;
}
