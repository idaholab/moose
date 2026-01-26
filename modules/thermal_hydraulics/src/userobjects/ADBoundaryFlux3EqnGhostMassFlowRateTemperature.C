//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnGhostMassFlowRateTemperature.h"
#include "THMIndicesVACE.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnGhostMassFlowRateTemperature);

InputParameters
ADBoundaryFlux3EqnGhostMassFlowRateTemperature::validParams()
{
  InputParameters params = ADBoundaryFlux3EqnGhostBase::validParams();

  params.addClassDescription(
      "Computes a boundary flux from a specified mass flow rate and temperature for the 1-D, "
      "1-phase, variable-area Euler equations using a ghost cell");

  params.addRequiredParam<Real>("mass_flow_rate", "Specified mass flow rate");
  params.addRequiredParam<Real>("T", "Specified temperature");
  params.addRequiredParam<std::vector<FunctionName>>(
      "passives", "Specified passive transport functions [amount/m^3]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of single-phase fluid properties user object");

  params.declareControllable("mass_flow_rate T");
  return params;
}

ADBoundaryFlux3EqnGhostMassFlowRateTemperature::ADBoundaryFlux3EqnGhostMassFlowRateTemperature(
    const InputParameters & parameters)
  : ADBoundaryFlux3EqnGhostBase(parameters),

    _rhouA(getParam<Real>("mass_flow_rate")),
    _T(getParam<Real>("T")),
    _reversible(getParam<bool>("reversible")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  // get specified passive transport functions
  const auto & passives = getParam<std::vector<FunctionName>>("passives");
  _n_passives = passives.size();
  _passives_fn.resize(_n_passives);
  for (const auto i : make_range(_n_passives))
    _passives_fn[i] = &getFunctionByName(passives[i]);
}

std::vector<ADReal>
ADBoundaryFlux3EqnGhostMassFlowRateTemperature::getGhostCellSolution(const std::vector<ADReal> & U,
                                                                     const Point & point) const
{
  const ADReal rhoA = U[THMVACE1D::RHOA];
  const ADReal rhouA = U[THMVACE1D::RHOUA];
  const ADReal rhoEA = U[THMVACE1D::RHOEA];
  const ADReal A = U[THMVACE1D::AREA];

  std::vector<ADReal> U_ghost(THMVACE1D::N_FLUX_INPUTS + _n_passives);
  if (!_reversible || THM::isInlet(_rhouA, _normal))
  {
    // Pressure is the only quantity coming from the interior
    const ADReal rho = rhoA / A;
    const ADReal vel = rhouA / rhoA;
    const ADReal E = rhoEA / rhoA;
    const ADReal e = E - 0.5 * vel * vel;
    const ADReal p = _fp.p_from_v_e(1.0 / rho, e);

    const ADReal rho_b = _fp.rho_from_p_T(p, _T);
    const ADReal vel_b = _rhouA / (rho_b * A);
    const ADReal e_b = _fp.e_from_p_rho(p, rho_b);
    const ADReal E_b = e_b + 0.5 * vel_b * vel_b;

    U_ghost[THMVACE1D::RHOA] = rho_b * A;
    U_ghost[THMVACE1D::RHOUA] = _rhouA;
    U_ghost[THMVACE1D::RHOEA] = rho_b * E_b * A;
    U_ghost[THMVACE1D::AREA] = A;
    for (const auto i : make_range(_n_passives))
      U_ghost[THMVACE1D::N_FLUX_INPUTS + i] = _passives_fn[i]->value(_t, point) * A;
  }
  else
  {
    U_ghost[THMVACE1D::RHOA] = rhoA;
    U_ghost[THMVACE1D::RHOUA] = _rhouA;
    U_ghost[THMVACE1D::RHOEA] = rhoEA;
    U_ghost[THMVACE1D::AREA] = A;
    for (const auto i : make_range(_n_passives))
      U_ghost[THMVACE1D::N_FLUX_INPUTS + i] = U[THMVACE1D::N_FLUX_INPUTS + i];
  }

  return U_ghost;
}
