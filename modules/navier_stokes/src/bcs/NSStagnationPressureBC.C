/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSStagnationPressureBC.h"

// Full specialization of the validParams function for this object
template<>
InputParameters validParams<NSStagnationPressureBC>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NSStagnationBC>();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "");

  // Required parameters
  params.addRequiredParam<Real>("desired_stagnation_pressure", "");

  return params;
}




// Constructor, be sure to call the base class constructor first!
NSStagnationPressureBC::NSStagnationPressureBC(const InputParameters & parameters)
    : NSStagnationBC(parameters),

      // Coupled variables
      _pressure(coupledValue("pressure")),

      // Required parameters
      _desired_stagnation_pressure(getParam<Real>("desired_stagnation_pressure"))
{}



// Specialization of the computeQpResidual() function for this class.
Real NSStagnationPressureBC::computeQpResidual()
{
  // The velocity vector
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Mach number, squared
  Real M2 = vel.norm_sq() / (_gamma * _R * _temperature[_qp]);

  // p_0 = p*(1 + 0.5*(gam-1)*M^2)^(gam/(gam-1))
  Real computed_stagnation_pressure = _pressure[_qp] * std::pow(1. + 0.5*(_gamma-1.)*M2, _gamma/(_gamma-1.));

  // Return the difference between the current solution's stagnation pressure
  // and the desired.  The Dirichlet condition asserts that these should be equal.
  return computed_stagnation_pressure - _desired_stagnation_pressure;
}


