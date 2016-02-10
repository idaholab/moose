/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSStagnationTemperatureBC.h"

// Full specialization of the validParams function for this object
template<>
InputParameters validParams<NSStagnationTemperatureBC>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSStagnationBC>();

  // Required parameters
  params.addRequiredParam<Real>("desired_stagnation_temperature", "");

  return params;
}




// Constructor, be sure to call the base class constructor first!
NSStagnationTemperatureBC::NSStagnationTemperatureBC(const InputParameters & parameters)
    : NSStagnationBC(parameters),

      // Required parameters
      _desired_stagnation_temperature(getParam<Real>("desired_stagnation_temperature"))
{}



// Specialization of the computeQpResidual() function for this class.
Real NSStagnationTemperatureBC::computeQpResidual()
{
  // The velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Mach number, squared
  Real M2 = vel.norm_sq() / (_gamma * _R * _temperature[_qp]);

  // T_0 = T*(1 + 0.5*(gam-1)*M^2)
  Real computed_stagnation_temperature = _temperature[_qp] * (1. + 0.5*(_gamma-1.)*M2);

  // Return the difference between the current solution's stagnation temperature
  // and the desired.  The Dirichlet condition asserts that these should be equal.
  return computed_stagnation_temperature - _desired_stagnation_temperature;
}


