//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnHLLC.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADNumericalFlux3EqnHLLC);

InputParameters
ADNumericalFlux3EqnHLLC::validParams()
{
  InputParameters params = ADNumericalFlux3EqnBase::validParams();
  params += NaNInterface::validParams();
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");
  params.addClassDescription("Computes internal side flux for the 1-D, 1-phase, variable-area "
                             "Euler equations using the HLLC approximate Riemann solver.");
  return params;
}

ADNumericalFlux3EqnHLLC::ADNumericalFlux3EqnHLLC(const InputParameters & parameters)
  : ADNumericalFlux3EqnBase(parameters),
    NaNInterface(this),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADNumericalFlux3EqnHLLC::calcFlux(const std::vector<ADReal> & U1,
                                  const std::vector<ADReal> & U2,
                                  const ADReal & nLR_dot_d,
                                  std::vector<ADReal> & FL,
                                  std::vector<ADReal> & FR) const
{
  // extract the conserved variables and area

  const ADReal rhoA1 = U1[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouA1 = U1[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEA1 = U1[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal A1 = U1[THM3Eqn::CONS_VAR_AREA];

  const ADReal rhoA2 = U2[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouA2 = U2[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEA2 = U2[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal A2 = U2[THM3Eqn::CONS_VAR_AREA];

  // reference transformation normal
  const ADReal & nx = nLR_dot_d;

  // compute the primitive variables

  const ADReal rho1 = rhoA1 / A1;
  const ADReal rhou1 = rhouA1 / A1;
  const ADReal rhoE1 = rhoEA1 / A1;
  const ADReal u1 = rhouA1 / rhoA1;
  const ADReal q1 = u1 * nx;
  const ADReal v1 = 1.0 / rho1;
  const ADReal E1 = rhoEA1 / rhoA1;
  const ADReal e1 = E1 - 0.5 * u1 * u1;
  const ADReal p1 = _fp.p_from_v_e(v1, e1);
  const ADReal H1 = E1 + p1 / rho1;
  const ADReal c1 = _fp.c_from_v_e(v1, e1);

  const ADReal rho2 = rhoA2 / A2;
  const ADReal rhou2 = rhouA2 / A2;
  const ADReal rhoE2 = rhoEA2 / A2;
  const ADReal u2 = rhouA2 / rhoA2;
  const ADReal q2 = u2 * nx;
  const ADReal v2 = 1.0 / rho2;
  const ADReal E2 = rhoEA2 / rhoA2;
  const ADReal e2 = E2 - 0.5 * u2 * u2;
  const ADReal p2 = _fp.p_from_v_e(v2, e2);
  const ADReal H2 = E2 + p2 / rho2;
  const ADReal c2 = _fp.c_from_v_e(v2, e2);

  // compute Roe-averaged variables
  const ADReal sqrt_rho1 = std::sqrt(rho1);
  const ADReal sqrt_rho2 = std::sqrt(rho2);
  const ADReal u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal q_roe = u_roe * nx;
  const ADReal H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / (sqrt_rho1 + sqrt_rho2);
  const ADReal h_roe = H_roe - 0.5 * u_roe * u_roe;
  const ADReal rho_roe = std::sqrt(rho1 * rho2);
  const ADReal v_roe = 1.0 / rho_roe;
  const ADReal e_roe = _fp.e_from_v_h(v_roe, h_roe);
  const ADReal c_roe = _fp.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  const ADReal s1 = std::min(q1 - c1, q_roe - c_roe);
  const ADReal s2 = std::max(q2 + c2, q_roe + c_roe);
  const ADReal sm = (rho2 * q2 * (s2 - q2) - rho1 * q1 * (s1 - q1) + p1 - p2) /
                    (rho2 * (s2 - q2) - rho1 * (s1 - q1));

  // compute Omega_L, Omega_R
  const ADReal omeg1 = 1.0 / (s1 - sm);
  const ADReal omeg2 = 1.0 / (s2 - sm);

  // compute p^*
  const ADReal ps = rho1 * (s1 - q1) * (sm - q1) + p1;

  // compute U_L^*, U_R^*

  const ADReal rhoLs = omeg1 * (s1 - q1) * rho1;
  const ADReal rhouLs = omeg1 * ((s1 - q1) * rhou1 + (ps - p1) * nx);
  const ADReal rhoELs = omeg1 * ((s1 - q1) * rhoE1 - p1 * q1 + ps * sm);

  const ADReal rhoRs = omeg2 * (s2 - q2) * rho2;
  const ADReal rhouRs = omeg2 * ((s2 - q2) * rhou2 + (ps - p2) * nx);
  const ADReal rhoERs = omeg2 * ((s2 - q2) * rhoE2 - p2 * q2 + ps * sm);

  const ADReal A_flow = computeFlowArea(U1, U2);

  // compute the fluxes
  FL.resize(THM3Eqn::N_EQ);
  if (s1 > 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = u1 * rho1 * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (u1 * rhou1 + p1) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = u1 * (rhoE1 + p1) * A_flow;

    _last_region_index = 0;
  }
  else if (s1 <= 0.0 && sm > 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = sm * nx * rhoLs * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (sm * nx * rhouLs + ps) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = sm * nx * (rhoELs + ps) * A_flow;

    _last_region_index = 1;
  }
  else if (sm <= 0.0 && s2 >= 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = sm * nx * rhoRs * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (sm * nx * rhouRs + ps) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = sm * nx * (rhoERs + ps) * A_flow;

    _last_region_index = 2;
  }
  else if (s2 < 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = u2 * rho2 * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (u2 * rhou2 + p2) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = u2 * (rhoE2 + p2) * A_flow;

    _last_region_index = 3;
  }
  else
  {
    FL = {getNaN(), getNaN(), getNaN()};
  }

  FR = FL;

  const ADReal A_wall_L = A1 - A_flow;
  FL[THM3Eqn::EQ_MOMENTUM] += p1 * A_wall_L;

  const ADReal A_wall_R = A2 - A_flow;
  FR[THM3Eqn::EQ_MOMENTUM] += p2 * A_wall_R;
}

ADReal
ADNumericalFlux3EqnHLLC::computeFlowArea(const std::vector<ADReal> & U1,
                                         const std::vector<ADReal> & U2) const
{
  return std::min(U1[THM3Eqn::CONS_VAR_AREA], U2[THM3Eqn::CONS_VAR_AREA]);
}
