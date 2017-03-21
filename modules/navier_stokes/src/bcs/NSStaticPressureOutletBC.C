/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSStaticPressureOutletBC.h"

template <>
InputParameters
validParams<NSStaticPressureOutletBC>()
{
  InputParameters params = validParams<MooseObject>();
  params.addClassDescription("This class facilitates adding specified static pressure outlet BCs "
                             "for the Euler equations.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  params.addRequiredParam<Real>("specified_pressure", "The specifed static pressure");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  // Must be called from every base MOOSE system to create linkage with the Action system.
  params.registerBase("NSStaticPressureOutletBC");

  return params;
}

NSStaticPressureOutletBC::NSStaticPressureOutletBC(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

NSStaticPressureOutletBC::~NSStaticPressureOutletBC() {}
