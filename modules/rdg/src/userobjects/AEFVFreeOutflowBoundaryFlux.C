/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVFreeOutflowBoundaryFlux.h"

template<>
InputParameters validParams<AEFVFreeOutflowBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();
  params.addClassDescription("Free outflow BC based boundary flux user object for the advection equation using a cell-centered finite volume method.");
  return params;
}

AEFVFreeOutflowBoundaryFlux::AEFVFreeOutflowBoundaryFlux(const InputParameters & parameters) :
    BoundaryFluxBase(parameters)
{
}

AEFVFreeOutflowBoundaryFlux::~AEFVFreeOutflowBoundaryFlux()
{
}

void
AEFVFreeOutflowBoundaryFlux::calcFlux(unsigned int /*iside*/,
                                      unsigned int /*ielem*/,
                                      const std::vector<Real> & uvec1,
                                      const std::vector<Real> & dwave,
                                      std::vector<Real> & flux) const
{
  // assume the velocity vector is constant, e.g. = (1., 1., 1.)
  Real uadv1 = 1.;
  Real vadv1 = 1.;
  Real wadv1 = 1.;

  // assign the size of flux vector, e.g. = 1 for the advection equation
  flux.resize(1);

  // finally calculate the flux
  flux[0] = (uadv1 * dwave[0] + vadv1 * dwave[1] + wadv1 * dwave[2]) * uvec1[0];
}

void
AEFVFreeOutflowBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                          unsigned int /*ielem*/,
                                          const std::vector<Real> & /*uvec1*/,
                                          const std::vector<Real> & /*dwave*/,
                                          DenseMatrix<Real> & /*jac1*/) const
{
}
