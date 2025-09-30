//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMPinPositions.h"

registerMooseObject("SubChannelApp", SCMPinPositions);

InputParameters
SCMPinPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Create positions at the pin positions in the SubChannel mesh.");

  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  // SCM mesh only defined on process 0
  params.set<bool>("auto_broadcast") = true;

  return params;
}

SCMPinPositions::SCMPinPositions(const InputParameters & parameters)
  : Positions(parameters), _scm_mesh(dynamic_cast<const SubChannelMesh *>(&_fe_problem.mesh()))
{
  // This makes this object half as useful. We should lift this
  if (!_scm_mesh)
    mooseError("Can only be used with a subchannel mesh at this time.");

  // Obtain the positions from the mesh
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
SCMPinPositions::initialize()
{
  clearPositions();
  // Add the pin positions
  for (const auto i_pin : make_range(_scm_mesh->getNumOfPins()))
    _positions.push_back(Point(*_scm_mesh->getPinNode(i_pin, 0)));

  _initialized = true;
}
