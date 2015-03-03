/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSImposedVelocityBC.h"

// Full specialization of the validParams function for this object
template<>
InputParameters validParams<NSImposedVelocityBC>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NodalBC>();

  // Declare variables as coupled...
  params.addRequiredCoupledVar("rho", "");

  // New style.  Note, addRequiredParam is a templated function, templated
  // on the type of the parameter being added. (Second string argument is optional documentation.)
  params.addRequiredParam<Real>("desired_velocity", "");// make it required!

  return params;
}




// Constructor, be sure to call the base class constructor first!
NSImposedVelocityBC::NSImposedVelocityBC(const std::string & name, InputParameters parameters)
  :NodalBC(name, parameters),
   _rho(coupledValue("rho")),
   _desired_velocity(getParam<Real>("desired_velocity"))
  {}



// Specialization of the computeQpResidual function for this class
Real NSImposedVelocityBC::computeQpResidual()
{
  // Return the difference between the current momentum and the desired value
  // (rho*u) - rho*desired_velocity
  return _u[_qp] - (_rho[_qp] * _desired_velocity);
}

