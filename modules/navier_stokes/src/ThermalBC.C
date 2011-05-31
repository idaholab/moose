#include "ThermalBC.h"

template<>
InputParameters validParams<ThermalBC>()
{
  InputParameters params = validParams<NodalBC>();

  // Declare some required coupled variables
  params.addRequiredCoupledVar("rho", "");

  params.addRequiredParam<Real>("initial", "Initial temperature");
  params.addRequiredParam<Real>("final", "Final temperature");
  params.addRequiredParam<Real>("duration", "Time over which temperature ramps up from initial to final");
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  return params;
}

ThermalBC::ThermalBC(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters),
   _rho_var(coupled("rho")),
   _rho(coupledValue("rho")),
   _initial(getParam<Real>("initial")),
   _final(getParam<Real>("final")),
   _duration(getParam<Real>("duration")),
   _cv(getParam<Real>("cv"))
  {}
  



Real
ThermalBC::computeQpResidual()
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

// std::ios_base::fmtflags flags = std::cout.flags();  
// std::cout << std::scientific << std::setprecision(16);
// std::cout << "rho*E                        =" << _u[_qp] << std::endl;
// std::cout << "(_p[_qp] * _c_v[_qp] * value)=" << (_p[_qp] * _c_v[_qp] * value) << std::endl;
// //std::cout << "_c_v[_qp]                    =" << _c_v[_qp] << std::endl;
// std::cout.flags(flags);

  return _u[_qp] - (_rho[_qp] * _cv * value);
}
