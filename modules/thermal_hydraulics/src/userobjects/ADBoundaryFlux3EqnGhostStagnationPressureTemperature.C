//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostStagnationPressureTemperature.h"
#include "SinglePhaseFluidProperties.h"
#include "THMIndicesVACE.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostStagnationPressureTemperature);

InputParameters
ADBoundaryFlux3EqnGhostStagnationPressureTemperature::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription("Computes boundary flux from a specified stagnation pressure and "
                             "temperature for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredParam<Real>("p0", "Stagnation pressure");
  params.addRequiredParam<Real>("T0", "Stagnation temperature");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  params.declareControllable("p0 T0");

  return params;
}

ADBoundaryFlux3EqnGhostStagnationPressureTemperature::
    ADBoundaryFlux3EqnGhostStagnationPressureTemperature(const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _p0(getParam<Real>("p0")),
    _T0(getParam<Real>("T0")),
    _reversible(getParam<bool>("reversible")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostStagnationPressureTemperature::getGhostCellSolution(
    const std::vector<ADReal> & U) const
{
  const ADReal rhoA = U[THMVACE1D::RHOA];
  const ADReal rhouA = U[THMVACE1D::RHOUA];
  const ADReal A = U[THMVACE1D::AREA];

  const ADReal vel = rhouA / rhoA;

  std::vector<ADReal> U_ghost(THMVACE1D::N_FLUX_INPUTS);
  if (!_reversible || THM::isInlet(vel, _normal))
  {
    // compute stagnation quantities
    const ADReal rho0 = _fp.rho_from_p_T(_p0, _T0);
    const ADReal e0 = _fp.e_from_p_rho(_p0, rho0);
    const ADReal v0 = 1.0 / rho0;
    const ADReal h0 = _fp.h_from_p_T(_p0, _T0);
    const ADReal s0 = _fp.s_from_v_e(v0, e0);

    // compute static quantities
    const ADReal h = h0 - 0.5 * vel * vel;
    const ADReal s = s0;
    const ADReal p = _fp.p_from_h_s(h, s);
    const ADReal rho = _fp.rho_from_p_s(p, s);
    const ADReal e = _fp.e_from_p_rho(p, rho);
    const ADReal E = e + 0.5 * vel * vel;

    U_ghost[THMVACE1D::RHOA] = rho * A;
    U_ghost[THMVACE1D::RHOUA] = rho * vel * A;
    U_ghost[THMVACE1D::RHOEA] = rho * E * A;
    U_ghost[THMVACE1D::AREA] = A;
  }
  else
  {
    const ADReal rho = rhoA / A;
    const ADReal E = _fp.e_from_p_rho(_p0, rho) + 0.5 * vel * vel;

    U_ghost[THMVACE1D::RHOA] = rhoA;
    U_ghost[THMVACE1D::RHOUA] = rhouA;
    U_ghost[THMVACE1D::RHOEA] = rhoA * E;
    U_ghost[THMVACE1D::AREA] = A;
  }

  return U_ghost;
}
