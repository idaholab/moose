#include "NSInflowThermalBC.h"

template<>
InputParameters validParams<NSInflowThermalBC>()
{
  InputParameters params = validParams<NodalBC>();

  // Global constant parameters
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  // Boundary condition values, all required except for velocity which defaults to zero.
  params.addRequiredParam<Real>("specified_rho", "Density of incoming flow");
  params.addRequiredParam<Real>("specified_temperature", "Temperature of incoming flow");
  params.addParam<Real>("specified_velocity_magnitude", 0., "Velocity magnitude of incoming flow");
  
  return params;
}




NSInflowThermalBC::NSInflowThermalBC(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters),
   _cv(getParam<Real>("cv")),
   _specified_rho(getParam<Real>("specified_rho")),
   _specified_temperature(getParam<Real>("specified_temperature")),
   _specified_velocity_magnitude(getParam<Real>("specified_velocity_magnitude"))
  {}




Real
NSInflowThermalBC::computeQpResidual()
{
  // For the total energy, the essential BC is: 
  // rho*E = rho*(c_v*T + 0.5*|u|^2)
  //
  // or, in residual form, (In general, this BC is coupled to the velocity variables.)
  // rho*E - rho*(c_v*T + 0.5*|u|^2) = 0
  //
  // ***at a no-slip wall*** this further reduces to (no coupling to velocity variables):
  // rho*E - rho*cv*T = 0

  return _u[_qp] - _specified_rho * (_cv * _specified_temperature + 0.5*_specified_velocity_magnitude*_specified_velocity_magnitude);
}
