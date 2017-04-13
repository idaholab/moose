/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
