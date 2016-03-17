/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSThermalBC.h"

template<>
InputParameters validParams<NSThermalBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredParam<Real>("initial", "Initial temperature");
  params.addRequiredParam<Real>("final", "Final temperature");
  params.addRequiredParam<Real>("duration", "Time over which temperature ramps up from initial to final");
  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");
  return params;
}

NSThermalBC::NSThermalBC(const InputParameters & parameters) :
    NodalBC(parameters),
    _rho_var(coupled("rho")),
    _rho(coupledValue("rho")),
    _initial(getParam<Real>("initial")),
    _final(getParam<Real>("final")),
    _duration(getParam<Real>("duration")),
    _R(getParam<Real>("R")),
    _gamma(getParam<Real>("gamma"))
{
}

Real
NSThermalBC::computeQpResidual()
{
  Real value;

  // For constant temperature, set _initial = _final, or set _duration=0 and set _final.
  //
  // T(t) = T_i + (T_f - T_i) * sin (pi/2 * t/t_d)
  if (_t < _duration)
    value = _initial + (_final - _initial) * std::sin((0.5* libMesh::pi)  * _t/_duration);
  else
    value = _final;

  // For the total energy, the essential BC is:
  // rho*E = rho*(c_v*T + |u|^2/2)
  //
  // or, in residual form, (In general, this BC is coupled to the velocity variables.)
  // rho*E - rho*(c_v*T + |u|^2/2) = 0
  //
  // ***at a no-slip wall*** this further reduces to (no coupling to velocity variables):
  // rho*E - rho*cv*T = 0

  // std::ios_base::fmtflags flags = Moose::out.flags();
  // Moose::out << std::scientific << std::setprecision(16);
  // Moose::out << "rho*E                        =" << _u[_qp] << std::endl;
  // Moose::out << "(_p[_qp] * _c_v[_qp] * value)=" << (_p[_qp] * _c_v[_qp] * value) << std::endl;
  // //Moose::out << "_c_v[_qp]                    =" << _c_v[_qp] << std::endl;
  // Moose::out.flags(flags);

  Real cv = _R / (_gamma - 1.0);
  return _u[_qp] - (_rho[_qp] * cv * value);
}
