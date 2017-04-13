/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSAction.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSAction>()
{
  InputParameters params = validParams<Action>();

  params.addClassDescription("This is a base Action class for the Navier-Stokes module which is "
                             "responsible for building lists of names that other Actions can "
                             "subsequently use.  Subclasses should call its act() function prior "
                             "to doing their own work.");
  return params;
}

NSAction::NSAction(InputParameters parameters) : Action(parameters), _dim(0) {}

NSAction::~NSAction() {}

void
NSAction::act()
{
  _vars.clear();
  _auxs.clear();

  _dim = _mesh->dimension();

  // Build up the vector of variable names for the user, depending on
  // the mesh dimension.
  _vars.push_back(NS::density);
  _vars.push_back(NS::momentum_x);
  if (_dim >= 2)
    _vars.push_back(NS::momentum_y);
  if (_dim >= 3)
    _vars.push_back(NS::momentum_z);
  _vars.push_back(NS::total_energy);

  // Add Aux variables.  These are all required in order for the code
  // to run, so they should not be independently selectable by the
  // user.
  _auxs.push_back(NS::velocity_x);
  if (_dim >= 2)
    _auxs.push_back(NS::velocity_y);
  if (_dim >= 3)
    _auxs.push_back(NS::velocity_z);

  _auxs.push_back(NS::pressure);
  _auxs.push_back(NS::temperature);
  _auxs.push_back(NS::enthalpy);
  _auxs.push_back(NS::mach_number);

  // Needed for FluidProperties calculations
  _auxs.push_back(NS::internal_energy);
  _auxs.push_back(NS::specific_volume);
}
