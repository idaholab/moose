/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSStagnationBC.h"
#include "MooseMesh.h"

// Full specialization of the validParams function for this object
template<>
InputParameters validParams<NSStagnationBC>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<NodalBC>();

  // Declare variables as coupled...
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("temperature", "");

  // Required parameters
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredParam<Real>("R", "Gas constant.");

  return params;
}




// Constructor, be sure to call the base class constructor first!
NSStagnationBC::NSStagnationBC(const InputParameters & parameters)
    : NodalBC(parameters),

      // Coupled variables
      _u_vel(coupledValue("u")),
      _v_vel(coupledValue("v")),
      _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),

      _temperature(coupledValue("temperature")),

      // Required parameters
      _gamma(getParam<Real>("gamma")),
      _R(getParam<Real>("R"))
{}




