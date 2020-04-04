//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVUpwindInternalSideFlux.h"

registerMooseObject("RdgApp", AEFVUpwindInternalSideFlux);

InputParameters
AEFVUpwindInternalSideFlux::validParams()
{
  InputParameters params = InternalSideFluxBase::validParams();
  params.addClassDescription("Upwind numerical flux scheme for the advection equation using a "
                             "cell-centered finite volume method.");
  params.addParam<Real>("velocity", 1.0, "Advective velocity");
  return params;
}

AEFVUpwindInternalSideFlux::AEFVUpwindInternalSideFlux(const InputParameters & parameters)
  : InternalSideFluxBase(parameters), _velocity(getParam<Real>("velocity"))
{
}

AEFVUpwindInternalSideFlux::~AEFVUpwindInternalSideFlux() {}

void
AEFVUpwindInternalSideFlux::calcFlux(unsigned int /*iside*/,
                                     dof_id_type /*ielem*/,
                                     dof_id_type /*ineig*/,
                                     const std::vector<Real> & uvec1,
                                     const std::vector<Real> & uvec2,
                                     const RealVectorValue & dwave,
                                     std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");
  mooseAssert(uvec2.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");

  // assign the size of flux vector, e.g. = 1 for the advection equation
  flux.resize(1);

  // assume a constant velocity on the left
  RealVectorValue uadv1(_velocity, 0.0, 0.0);

  // assume a constant velocity on the right
  RealVectorValue uadv2(_velocity, 0.0, 0.0);

  // normal velocity on the left and right
  Real vdon1 = uadv1 * dwave;
  Real vdon2 = uadv2 * dwave;

  // calculate the so-called a^plus and a^minus
  Real aplus = 0.5 * (vdon1 + std::abs(vdon1));
  Real amins = 0.5 * (vdon2 - std::abs(vdon2));

  // finally calculate the flux
  flux[0] = aplus * uvec1[0] + amins * uvec2[0];
}

void
AEFVUpwindInternalSideFlux::calcJacobian(unsigned int /*iside*/,
                                         dof_id_type /*ielem*/,
                                         dof_id_type /*ineig*/,
                                         const std::vector<Real> & libmesh_dbg_var(uvec1),
                                         const std::vector<Real> & libmesh_dbg_var(uvec2),
                                         const RealVectorValue & dwave,
                                         DenseMatrix<Real> & jac1,
                                         DenseMatrix<Real> & jac2) const
{
  mooseAssert(uvec1.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");
  mooseAssert(uvec2.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");

  // assign the size of Jacobian matrix, e.g. = (1, 1) for the advection equation
  jac1.resize(1, 1);
  jac2.resize(1, 1);

  // assume a constant velocity on the left
  RealVectorValue uadv1(_velocity, 0.0, 0.0);

  // assume a constant velocity on the right
  RealVectorValue uadv2(_velocity, 0.0, 0.0);

  // normal velocity on the left and right
  Real vdon1 = uadv1 * dwave;
  Real vdon2 = uadv2 * dwave;

  // calculate the so-called a^plus and a^minus
  Real aplus = 0.5 * (vdon1 + std::abs(vdon1));
  Real amins = 0.5 * (vdon2 - std::abs(vdon2));

  // finally calculate the Jacobian matrix
  jac1(0, 0) = aplus;
  jac2(0, 0) = amins;
}
