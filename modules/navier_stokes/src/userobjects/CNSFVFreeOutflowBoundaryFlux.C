/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVFreeOutflowBoundaryFlux.h"

template <>
InputParameters
validParams<CNSFVFreeOutflowBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("A user object that computes the outflow boundary flux.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVFreeOutflowBoundaryFlux::CNSFVFreeOutflowBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters), _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

CNSFVFreeOutflowBoundaryFlux::~CNSFVFreeOutflowBoundaryFlux() {}

void
CNSFVFreeOutflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                       dof_id_type /*ielem*/,
                                       const std::vector<Real> & uvec1,
                                       const RealVectorValue & dwave,
                                       std::vector<Real> & flux) const
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

  /// assign the proper size for the flux vector

  flux.resize(5);

  /// derived variables on the left

  Real uadv1 = rhou1 / rho1;
  Real vadv1 = rhov1 / rho1;
  Real wadv1 = rhow1 / rho1;
  Real v = 1. / rho1;
  Real e = rhoe1 / rho1 - 0.5 * (uadv1 * uadv1 + vadv1 * vadv1 + wadv1 * wadv1);
  Real pres1 = _fp.pressure(v, e);

  Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;

  flux[0] = vdon1 * rho1;
  flux[1] = vdon1 * rhou1 + pres1 * nx;
  flux[2] = vdon1 * rhov1 + pres1 * ny;
  flux[3] = vdon1 * rhow1 + pres1 * nz;
  flux[4] = vdon1 * (rhoe1 + pres1);
}

void
CNSFVFreeOutflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                           dof_id_type /*ielem*/,
                                           const std::vector<Real> & /*uvec1*/,
                                           const RealVectorValue & /*dwave*/,
                                           DenseMatrix<Real> & /*jac1*/) const
{
}
