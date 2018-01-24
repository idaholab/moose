//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSNoPenetrationBC.h"

template <>
InputParameters
validParams<NSNoPenetrationBC>()
{
  InputParameters params = validParams<MooseObject>();
  params.addClassDescription(
      "This class facilitates adding solid wall 'no penetration' BCs for the Euler equations.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  // Must be called from every base MOOSE system to create linkage with the Action system.
  params.registerBase("NSNoPenetrationBC");

  return params;
}

NSNoPenetrationBC::NSNoPenetrationBC(const InputParameters & parameters) : MooseObject(parameters)
{
}

NSNoPenetrationBC::~NSNoPenetrationBC() {}
