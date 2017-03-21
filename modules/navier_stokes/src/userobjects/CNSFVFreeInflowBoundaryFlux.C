/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVFreeInflowBoundaryFlux.h"

template <>
InputParameters
validParams<CNSFVFreeInflowBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("A user object that computes the inflow boundary flux.");

  params.addRequiredParam<UserObjectName>("bc_uo", "Name for boundary condition user object");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVFreeInflowBoundaryFlux::CNSFVFreeInflowBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _bc_uo(getUserObject<BCUserObject>("bc_uo")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

CNSFVFreeInflowBoundaryFlux::~CNSFVFreeInflowBoundaryFlux() {}

void
CNSFVFreeInflowBoundaryFlux::calcFlux(unsigned int iside,
                                      dof_id_type ielem,
                                      const std::vector<Real> & uvec1,
                                      const RealVectorValue & dwave,
                                      std::vector<Real> & flux) const
{
  /// pass the inputs to local

  Real nx = dwave(0);
  Real ny = dwave(1);
  Real nz = dwave(2);

  /// assign the proper size for the flux vector

  flux.resize(5);

  std::vector<Real> U2(5, 0.);

  U2 = _bc_uo.getGhostCellValue(iside, ielem, uvec1, dwave);

  Real rho2 = U2[0];
  Real rhou2 = U2[1];
  Real rhov2 = U2[2];
  Real rhow2 = U2[3];
  Real rhoe2 = U2[4];

  Real uadv2 = rhou2 / rho2;
  Real vadv2 = rhov2 / rho2;
  Real wadv2 = rhow2 / rho2;
  Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
  Real v2 = 1. / rho2;
  Real e2 = rhoe2 / rho2 - 0.5 * vdov2;
  Real pres2 = _fp.pressure(v2, e2);

  Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;

  flux[0] = vdon2 * rho2;
  flux[1] = vdon2 * rho2 * uadv2 + pres2 * nx;
  flux[2] = vdon2 * rho2 * vadv2 + pres2 * ny;
  flux[3] = vdon2 * rho2 * wadv2 + pres2 * nz;
  flux[4] = vdon2 * (rhoe2 + pres2);
}

void
CNSFVFreeInflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                          dof_id_type /*ielem*/,
                                          const std::vector<Real> & /*uvec1*/,
                                          const RealVectorValue & /*dwave*/,
                                          DenseMatrix<Real> & /*jac1*/) const
{
}
