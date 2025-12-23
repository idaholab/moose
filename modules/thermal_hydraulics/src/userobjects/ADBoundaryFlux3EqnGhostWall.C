//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostWall.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostWall);

InputParameters
ADBoundaryFlux3EqnGhostWall::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription(
      "Wall boundary conditions for the 1-D, 1-phase, variable-area Euler equations");

  return params;
}

ADBoundaryFlux3EqnGhostWall::ADBoundaryFlux3EqnGhostWall(const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters)
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostWall::getGhostCellSolution(const std::vector<ADReal> & U1,
                                                  const Point & /*point*/) const
{
  std::vector<ADReal> U_ghost = U1;
  U_ghost[THMVACE1D::RHOUA] = -U1[THMVACE1D::RHOUA];

  return U_ghost;
}
