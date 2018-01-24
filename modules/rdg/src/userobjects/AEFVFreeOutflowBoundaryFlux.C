//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVFreeOutflowBoundaryFlux.h"

template <>
InputParameters
validParams<AEFVFreeOutflowBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();
  params.addClassDescription("Free outflow BC based boundary flux user object for the advection "
                             "equation using a cell-centered finite volume method.");
  return params;
}

AEFVFreeOutflowBoundaryFlux::AEFVFreeOutflowBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters)
{
}

AEFVFreeOutflowBoundaryFlux::~AEFVFreeOutflowBoundaryFlux() {}

void
AEFVFreeOutflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                      dof_id_type /*ielem*/,
                                      const std::vector<Real> & uvec1,
                                      const RealVectorValue & dwave,
                                      std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");

  // assume the velocity vector is constant, e.g. = (1., 1., 1.)
  RealVectorValue uadv1(1.0, 1.0, 1.0);

  // assign the size of flux vector, e.g. = 1 for the advection equation
  flux.resize(1);

  // finally calculate the flux
  flux[0] = (uadv1 * dwave) * uvec1[0];
}

void
AEFVFreeOutflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                          dof_id_type /*ielem*/,
                                          const std::vector<Real> & libmesh_dbg_var(uvec1),
                                          const RealVectorValue & /*dwave*/,
                                          DenseMatrix<Real> & /*jac1*/) const
{
  mooseAssert(uvec1.size() == 1, "Invalid size for uvec1. Must be single variable coupling.");
}
