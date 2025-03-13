//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFluxGasMixGhostWall.h"
#include "THMIndicesGasMix.h"

registerMooseObject("ThermalHydraulicsApp", BoundaryFluxGasMixGhostWall);

InputParameters
BoundaryFluxGasMixGhostWall::validParams()
{
  InputParameters params = BoundaryFluxGasMixGhostBase::validParams();

  params.addClassDescription("Wall boundary flux for FlowModelGasMix.");

  return params;
}

BoundaryFluxGasMixGhostWall::BoundaryFluxGasMixGhostWall(const InputParameters & parameters)
  : BoundaryFluxGasMixGhostBase(parameters)
{
}

std::vector<ADReal>
BoundaryFluxGasMixGhostWall::getGhostCellSolution(const std::vector<ADReal> & U1) const
{
  std::vector<ADReal> U_ghost(THMGasMix1D::N_FLUX_INPUTS);
  U_ghost[THMGasMix1D::XIRHOA] = U1[THMGasMix1D::XIRHOA];
  U_ghost[THMGasMix1D::RHOA] = U1[THMGasMix1D::RHOA];
  U_ghost[THMGasMix1D::RHOUA] = -U1[THMGasMix1D::RHOUA];
  U_ghost[THMGasMix1D::RHOEA] = U1[THMGasMix1D::RHOEA];
  U_ghost[THMGasMix1D::AREA] = U1[THMGasMix1D::AREA];

  return U_ghost;
}
