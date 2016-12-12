/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVUpwindInternalSideFlux.h"

template<>
InputParameters validParams<AEFVUpwindInternalSideFlux>()
{
  InputParameters params = validParams<InternalSideFluxBase>();
  params.addClassDescription("Upwind numerical flux scheme for the advection equation using a cell-centered finite volume method.");
  return params;
}

AEFVUpwindInternalSideFlux::AEFVUpwindInternalSideFlux(const InputParameters & parameters) :
    InternalSideFluxBase(parameters)
{
}

AEFVUpwindInternalSideFlux::~AEFVUpwindInternalSideFlux()
{
}

void
AEFVUpwindInternalSideFlux::calcFlux(unsigned int iside,
                                     unsigned int ielem,
                                     unsigned int ineig,
                                     const std::vector<Real> & uvec1,
                                     const std::vector<Real> & uvec2,
                                     const std::vector<Real> & dwave,
                                     std::vector<Real> & flux) const
{
  // assign the size of flux vector, e.g. = 1 for the advection equation
  flux.resize(1);

  // assume a constant velocity on the left
  Real uadv1 = 1.;
  Real vadv1 = 1.;
  Real wadv1 = 1.;

  // assume a constant velocity on the right
  Real uadv2 = 1.;
  Real vadv2 = 1.;
  Real wadv2 = 1.;

  // normal velocity on the left and right
  Real vdon1 = uadv1 * dwave[0] + vadv1 * dwave[1] + wadv1 * dwave[2];
  Real vdon2 = uadv2 * dwave[0] + vadv2 * dwave[1] + wadv2 * dwave[2];

  // calculate the so-called a^plus and a^minus
  Real aplus = 0.5 * (vdon1 + std::abs(vdon1));
  Real amins = 0.5 * (vdon2 - std::abs(vdon2));

  // finally calculate the flux
  flux[0] = aplus * uvec1[0] + amins * uvec2[0];
}

void
AEFVUpwindInternalSideFlux::calcJacobian(unsigned int iside,
                                         unsigned int ielem,
                                         unsigned int ineig,
                                         const std::vector<Real> & uvec1,
                                         const std::vector<Real> & uvec2,
                                         const std::vector<Real> & dwave,
                                         DenseMatrix<Real> & jac1,
                                         DenseMatrix<Real> & jac2) const
{
  // assign the size of Jacobian matrix, e.g. = (1, 1) for the advection equation
  jac1.resize(1, 1);
  jac2.resize(1, 1);

  // assume a constant velocity on the left
  Real uadv1 = 1.;
  Real vadv1 = 1.;
  Real wadv1 = 1.;

  // assume a constant velocity on the right
  Real uadv2 = 1.;
  Real vadv2 = 1.;
  Real wadv2 = 1.;

  // normal velocity on the left and right
  Real vdon1 = uadv1 * dwave[0] + vadv1 * dwave[1] + wadv1 * dwave[2];
  Real vdon2 = uadv2 * dwave[0] + vadv2 * dwave[1] + wadv2 * dwave[2];

  // calculate the so-called a^plus and a^minus
  Real aplus = 0.5 * (vdon1 + std::abs(vdon1));
  Real amins = 0.5 * (vdon2 - std::abs(vdon2));

  // finally calculate the Jacobian matrix
  jac1(0, 0) = aplus;
  jac2(0, 0) = amins;
}
