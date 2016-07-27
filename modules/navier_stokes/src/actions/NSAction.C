/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSAction.h"

// MOOSE includes
#include "MooseMesh.h"

template<>
InputParameters validParams<NSAction>()
{
  InputParameters params = validParams<Action>();

  return params;
}

NSAction::NSAction(InputParameters parameters) :
    Action(parameters),
    _dim(0)
{
}

NSAction::~NSAction()
{
}

void
NSAction::act()
{
  _vars.clear();
  _auxs.clear();

  _dim = _mesh->dimension();

  // Build up the vector of variable names for the user, depending on
  // the mesh dimension.
  _vars.push_back("rho");
  _vars.push_back("rhou");
  if (_dim >= 2)
    _vars.push_back("rhov");
  if (_dim >= 3)
    _vars.push_back("rhow");
  _vars.push_back("rhoE");

  // Add Aux variables.  These are all required in order for the code
  // to run, so they should not be independently selectable by the
  // user.
  _auxs.push_back("vel_x");
  if (_dim >= 2)
    _auxs.push_back("vel_y");
  if (_dim >= 3)
    _auxs.push_back("vel_z");

  _auxs.push_back("pressure");
  _auxs.push_back("temperature");
  _auxs.push_back("enthalpy");
  _auxs.push_back("Mach");

  // Needed for FluidProperties calculations
  _auxs.push_back("internal_energy");
  _auxs.push_back("specific_volume");
}
