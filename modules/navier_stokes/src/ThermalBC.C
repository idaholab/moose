#include "ThermalBC.h"

template<>
InputParameters validParams<ThermalBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.set<Real>("initial")=0.0;
  params.set<Real>("final")=0.0;
  params.set<Real>("duration")=0.0;
  return params;
}

ThermalBC::ThermalBC(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :BoundaryCondition(name, moose_system, setIntegratedParam(parameters, true)),
    _p_var(coupled("p")),
    _p(coupledVal("p")),
    _u_vel_var(coupled("u")),
    _u_vel(coupledVal("u")),
    _v_vel_var(coupled("v")),
    _v_vel(coupledVal("v")),
    _w_vel_var(_dim == 3 ? coupled("w") : 0),
    _w_vel(_dim == 3 ? coupledVal("w") : _zero),
    _initial(_parameters.get<Real>("initial")),
    _final(_parameters.get<Real>("final")),
    _duration(_parameters.get<Real>("duration"))
  {}

Real
ThermalBC::temperature()
{
  //Only constant material properties are allowed in a BC
  Real & R = _material->getConstantRealProperty("R");
  Real & c_v = _material->getConstantRealProperty("c_v");
  Real & gamma = _material->getConstantRealProperty("gamma");
  
  Real value = 1.0/c_v;

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
