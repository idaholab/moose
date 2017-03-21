/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVFreeInflowBCUserObject.h"

template <>
InputParameters
validParams<CNSFVFreeInflowBCUserObject>()
{
  InputParameters params = validParams<BCUserObject>();

  params.addClassDescription("A user object that computes the ghost cell values based on the free "
                             "inflow boundary condition.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addRequiredParam<Real>("infinity_density", "Infinity density");

  params.addRequiredParam<Real>("infinity_x_velocity",
                                "Infinity velocity component in x-direction");

  params.addParam<Real>("infinity_y_velocity", 0., "Infinity velocity component in y-direction");

  params.addParam<Real>("infinity_z_velocity", 0., "Infinity velocity component in z-direction");

  params.addRequiredParam<Real>("infinity_pressure", "Infinity pressure");

  return params;
}

CNSFVFreeInflowBCUserObject::CNSFVFreeInflowBCUserObject(const InputParameters & parameters)
  : BCUserObject(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _inf_rho(getParam<Real>("infinity_density")),
    _inf_uadv(getParam<Real>("infinity_x_velocity")),
    _inf_vadv(getParam<Real>("infinity_y_velocity")),
    _inf_wadv(getParam<Real>("infinity_z_velocity")),
    _inf_pres(getParam<Real>("infinity_pressure"))
{
}

std::vector<Real>
CNSFVFreeInflowBCUserObject::getGhostCellValue(unsigned int /*iside*/,
                                               dof_id_type /*ielem*/,
                                               const std::vector<Real> & /*uvec1*/,
                                               const RealVectorValue & /*dwave*/) const
{
  std::vector<Real> urigh(5, 0.);

  urigh[0] = _inf_rho;
  urigh[1] = _inf_rho * _inf_uadv;
  urigh[2] = _inf_rho * _inf_vadv;
  urigh[3] = _inf_rho * _inf_wadv;
  urigh[4] =
      _inf_rho * (_fp.e(_inf_pres, _inf_rho) +
                  0.5 * (_inf_uadv * _inf_uadv + _inf_vadv * _inf_vadv + _inf_wadv * _inf_wadv));

  return urigh;
}
