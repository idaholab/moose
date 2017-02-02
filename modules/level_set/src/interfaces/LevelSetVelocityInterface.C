/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "LevelSetVelocityInterface.h"

template<>
InputParameters
validParams<LevelSetVelocityInterface<> >()
{
  InputParameters parameters = emptyInputParameters();
  parameters.addCoupledVar("velocity_x", 0, "The variable containing the x-component of the velocity front.");
  parameters.addCoupledVar("velocity_y", 0, "The variable containing the y-component of the velocity front.");
  parameters.addCoupledVar("velocity_z", 0, "The variable containing the z-component of the velocity front.");
  return parameters;
}
