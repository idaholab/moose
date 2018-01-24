//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSWeakStagnationInletBC.h"

template <>
InputParameters
validParams<NSWeakStagnationInletBC>()
{
  InputParameters params = validParams<MooseObject>();
  params.addClassDescription("This class facilitates adding weak stagnation inlet BCs via an "
                             "Action by setting up the required parameters.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addRequiredParam<Real>("stagnation_pressure", "The specifed stagnation pressure");
  params.addRequiredParam<Real>("stagnation_temperature", "The specifed stagnation temperature");
  params.addRequiredParam<Real>("sx", "x-component of specifed flow direction");
  params.addParam<Real>(
      "sy", 0.0, "y-component of specifed flow direction"); // only required in >= 2D
  params.addParam<Real>("sz", 0.0, "z-component of specifed flow direction"); // only required in 3D
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  // Must be called from every base MOOSE system to create linkage with the Action system.
  params.registerBase("NSWeakStagnationInletBC");

  return params;
}

NSWeakStagnationInletBC::NSWeakStagnationInletBC(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

NSWeakStagnationInletBC::~NSWeakStagnationInletBC() {}
