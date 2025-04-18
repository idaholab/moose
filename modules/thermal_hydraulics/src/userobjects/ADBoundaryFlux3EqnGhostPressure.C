//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostPressure.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesVACE.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostPressure);

InputParameters
ADBoundaryFlux3EqnGhostPressure::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from a specified pressure for the 1-D, "
                             "1-phase, variable-area Euler equations");

  params.addRequiredParam<Real>("p", "Pressure");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  params.declareControllable("p");
  return params;
}

ADBoundaryFlux3EqnGhostPressure::ADBoundaryFlux3EqnGhostPressure(const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _p(getParam<Real>("p")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostPressure::getGhostCellSolution(const std::vector<ADReal> & U) const
{
  const ADReal rhoA = U[THMVACE1D::RHOA];
  const ADReal rhouA = U[THMVACE1D::RHOUA];
  const ADReal A = U[THMVACE1D::AREA];

  const ADReal rho = rhoA / A;
  const ADReal vel = rhouA / rhoA;
  const ADReal E = _fp.e_from_p_rho(_p, rho) + 0.5 * vel * vel;

  std::vector<ADReal> U_ghost(THMVACE1D::N_FLUX_INPUTS);
  U_ghost[THMVACE1D::RHOA] = rhoA;
  U_ghost[THMVACE1D::RHOUA] = rhouA;
  U_ghost[THMVACE1D::RHOEA] = rhoA * E;
  U_ghost[THMVACE1D::AREA] = A;

  return U_ghost;
}
