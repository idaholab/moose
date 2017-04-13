/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVFreeOutflowBCUserObject.h"

template <>
InputParameters
validParams<CNSFVFreeOutflowBCUserObject>()
{
  InputParameters params = validParams<BCUserObject>();

  params.addClassDescription("A user object that computes the ghost cell values based on the free "
                             "outflow boundary condition.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVFreeOutflowBCUserObject::CNSFVFreeOutflowBCUserObject(const InputParameters & parameters)
  : BCUserObject(parameters), _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
CNSFVFreeOutflowBCUserObject::getGhostCellValue(unsigned int /*iside*/,
                                                dof_id_type /*ielem*/,
                                                const std::vector<Real> & uvec1,
                                                const RealVectorValue & /*dwave*/) const
{
  return uvec1;
}
