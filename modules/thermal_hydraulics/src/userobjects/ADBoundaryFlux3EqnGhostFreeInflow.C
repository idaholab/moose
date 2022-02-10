//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostFreeInflow.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostFreeInflow);

InputParameters
ADBoundaryFlux3EqnGhostFreeInflow::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Free inflow boundary conditions from a ghost cell for the 1-D, "
                             "1-phase, variable-area Euler equations");

  params.addRequiredParam<Real>("rho_infinity", "Far-stream density value");
  params.addRequiredParam<Real>("vel_infinity", "Far-stream velocity value");
  params.addRequiredParam<Real>("p_infinity", "Far-stream pressure value");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

ADBoundaryFlux3EqnGhostFreeInflow::ADBoundaryFlux3EqnGhostFreeInflow(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _rho_inf(getParam<Real>("rho_infinity")),
    _vel_inf(getParam<Real>("vel_infinity")),
    _p_inf(getParam<Real>("p_infinity")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostFreeInflow::getGhostCellSolution(const std::vector<ADReal> & U1) const
{
  const ADReal A = U1[THM3Eqn::CONS_VAR_AREA];

  const ADReal e_inf = _fp.e_from_p_rho(_p_inf, _rho_inf);
  const ADReal E_inf = e_inf + 0.5 * _vel_inf * _vel_inf;

  const ADReal rhoA = _rho_inf * A;
  const ADReal rhouA = rhoA * _vel_inf;
  const ADReal rhoEA = rhoA * E_inf;

  std::vector<ADReal> U_ghost(THM3Eqn::N_CONS_VAR);
  U_ghost[THM3Eqn::CONS_VAR_RHOA] = rhoA;
  U_ghost[THM3Eqn::CONS_VAR_RHOUA] = rhouA;
  U_ghost[THM3Eqn::CONS_VAR_RHOEA] = rhoEA;
  U_ghost[THM3Eqn::CONS_VAR_AREA] = A;

  return U_ghost;
}
