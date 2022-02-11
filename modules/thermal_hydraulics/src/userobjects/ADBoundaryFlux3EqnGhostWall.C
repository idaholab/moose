//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostWall.h"
#include "THMIndices3Eqn.h"

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
ADBoundaryFlux3EqnGhostWall::getGhostCellSolution(const std::vector<ADReal> & U1) const
{
  std::vector<ADReal> U_ghost(THM3Eqn::N_CONS_VAR);
  U_ghost[THM3Eqn::CONS_VAR_RHOA] = U1[THM3Eqn::CONS_VAR_RHOA];
  U_ghost[THM3Eqn::CONS_VAR_RHOUA] = -U1[THM3Eqn::CONS_VAR_RHOUA];
  U_ghost[THM3Eqn::CONS_VAR_RHOEA] = U1[THM3Eqn::CONS_VAR_RHOEA];
  U_ghost[THM3Eqn::CONS_VAR_AREA] = U1[THM3Eqn::CONS_VAR_AREA];

  return U_ghost;
}
