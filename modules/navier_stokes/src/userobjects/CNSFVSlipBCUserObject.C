/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVSlipBCUserObject.h"

template <>
InputParameters
validParams<CNSFVSlipBCUserObject>()
{
  InputParameters params = validParams<BCUserObject>();

  params.addClassDescription("A user object that computes the ghost cell values based on the slip "
                             "wall boundary condition.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVSlipBCUserObject::CNSFVSlipBCUserObject(const InputParameters & parameters)
  : BCUserObject(parameters), _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<Real>
CNSFVSlipBCUserObject::getGhostCellValue(unsigned int /*iside*/,
                                         dof_id_type /*ielem*/,
                                         const std::vector<Real> & uvec1,
                                         const RealVectorValue & dwave) const
{
  /// pass the inputs to local

  Real rho1 = uvec1[0];
  Real rhou1 = uvec1[1];
  Real rhov1 = uvec1[2];
  Real rhow1 = uvec1[3];
  Real rhoe1 = uvec1[4];

  Real nx = dwave(0);
  Real ny = dwave(1);
  Real nz = dwave(2);

  std::vector<Real> urigh(5, 0.);

  Real mdotn = rhou1 * nx + rhov1 * ny + rhow1 * nz;

  urigh[0] = rho1;
  urigh[1] = rhou1 - 2. * mdotn * nx;
  urigh[2] = rhov1 - 2. * mdotn * ny;
  urigh[3] = rhow1 - 2. * mdotn * nz;
  urigh[4] = rhoe1;

  return urigh;
}
