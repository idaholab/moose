/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVCharacteristicBCUserObject.h"

template <>
InputParameters
validParams<CNSFVCharacteristicBCUserObject>()
{
  InputParameters params = validParams<BCUserObject>();

  params.addClassDescription("A user object that computes the ghost cell values based on the "
                             "characteristic boundary condition.");

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

CNSFVCharacteristicBCUserObject::CNSFVCharacteristicBCUserObject(const InputParameters & parameters)
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
CNSFVCharacteristicBCUserObject::getGhostCellValue(unsigned int iside,
                                                   dof_id_type ielem,
                                                   const std::vector<Real> & uvec1,
                                                   const RealVectorValue & /*dwave*/) const
{
  /// pass the inputs to local

  Real rho1 = uvec1[0];
  Real rhou1 = uvec1[1];
  Real rhov1 = uvec1[2];
  Real rhow1 = uvec1[3];
  Real rhoe1 = uvec1[4];

  std::vector<Real> urigh(5, 0.);

  /// compute the local Mach number with state variables on the left

  Real rhom1 = 1. / rho1;
  Real uadv1 = rhou1 * rhom1;
  Real vadv1 = rhov1 * rhom1;
  Real wadv1 = rhow1 * rhom1;
  Real vdov1 = uadv1 * uadv1 + vadv1 * vadv1 + wadv1 * wadv1;
  Real eint1 = rhoe1 * rhom1 - 0.5 * vdov1;
  Real mach1 = std::sqrt(vdov1) / _fp.c(rhom1, eint1);

  if (mach1 > -1. && mach1 < 0.)
  {
    /// subsonic inflow case
    ///   -- fix pressure (but also OK to extrapolate pressure)
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    urigh[0] = _inf_rho;
    urigh[1] = _inf_rho * _inf_uadv;
    urigh[2] = _inf_rho * _inf_vadv;
    urigh[3] = _inf_rho * _inf_wadv;
    urigh[4] =
        _inf_rho * (_fp.e(_inf_pres, _inf_rho) +
                    0.5 * (_inf_uadv * _inf_uadv + _inf_vadv * _inf_vadv + _inf_wadv * _inf_wadv));
  }
  else if (mach1 >= 0. && mach1 < 1.)
  {
    /// subsonic outflow case
    ///   -- fix pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocities
    ///   -- extrapolate mass fractions

    urigh[0] = rho1;
    urigh[1] = rhou1;
    urigh[2] = rhov1;
    urigh[3] = rhow1;
    urigh[4] = rho1 * _fp.e(_inf_pres, rho1) +
               0.5 * (rhou1 * rhou1 + rhov1 * rhov1 + rhow1 * rhow1) / rho1;
  }
  else if (mach1 <= -1.)
  {
    /// supersonic inflow case
    ///   -- fix pressure
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    urigh[0] = _inf_rho;
    urigh[1] = _inf_rho * _inf_uadv;
    urigh[2] = _inf_rho * _inf_vadv;
    urigh[3] = _inf_rho * _inf_wadv;
    urigh[4] =
        _inf_rho * (_fp.e(_inf_pres, _inf_rho) +
                    0.5 * (_inf_uadv * _inf_uadv + _inf_vadv * _inf_vadv + _inf_wadv * _inf_wadv));
  }
  else if (mach1 >= 1.)
  {
    /// supersonic outflow case
    ///   -- extrapolate pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocity
    ///   -- extrapolate mass fractions

    urigh[0] = rho1;
    urigh[1] = rhou1;
    urigh[2] = rhov1;
    urigh[3] = rhow1;
    urigh[4] = rhoe1;
  }
  else
    mooseError("Something is wrong in ",
               name(),
               ": ",
               __FUNCTION__,
               "\n",
               "ielem = ",
               ielem,
               "\n",
               "iside = ",
               iside,
               "\n",
               "mach1 = ",
               mach1,
               "\n");

  return urigh;
}
