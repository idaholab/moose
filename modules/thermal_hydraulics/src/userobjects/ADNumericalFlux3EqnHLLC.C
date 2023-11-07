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
ADNumericalFlux3EqnHLLC::calcFlux(const std::vector<ADReal> & UL,
                                  const std::vector<ADReal> & UR,
                                  const ADReal & nLR_dot_d,
                                  std::vector<ADReal> & FL,
                                  std::vector<ADReal> & FR) const
{
  // extract the conserved variables and area

  const ADReal rhoAL = UL[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouAL = UL[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEAL = UL[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal AL = UL[THM3Eqn::CONS_VAR_AREA];

  const ADReal rhoAR = UR[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouAR = UR[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEAR = UR[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal AR = UR[THM3Eqn::CONS_VAR_AREA];

  // reference transformation normal
  const ADReal & nx = nLR_dot_d;

  // compute the primitive variables

  const ADReal rhoL = rhoAL / AL;
  const ADReal rhouL = rhouAL / AL;
  const ADReal rhoEL = rhoEAL / AL;
  const ADReal uL = rhouAL / rhoAL;
  const ADReal unL = uL * nx;
  const ADReal vL = 1.0 / rhoL;
  const ADReal EL = rhoEAL / rhoAL;
  const ADReal eL = EL - 0.5 * uL * uL;
  const ADReal pL = _fp.p_from_v_e(vL, eL);
  const ADReal HL = EL + pL / rhoL;
  const ADReal cL = _fp.c_from_v_e(vL, eL);

  const ADReal rhoR = rhoAR / AR;
  const ADReal rhouR = rhouAR / AR;
  const ADReal rhoER = rhoEAR / AR;
  const ADReal uR = rhouAR / rhoAR;
  const ADReal unR = uR * nx;
  const ADReal vR = 1.0 / rhoR;
  const ADReal ER = rhoEAR / rhoAR;
  const ADReal eR = ER - 0.5 * uR * uR;
  const ADReal pR = _fp.p_from_v_e(vR, eR);
  const ADReal HR = ER + pR / rhoR;
  const ADReal cR = _fp.c_from_v_e(vR, eR);

  // compute Roe-averaged variables
  const ADReal sqrt_rhoL = std::sqrt(rhoL);
  const ADReal sqrt_rhoR = std::sqrt(rhoR);
  const ADReal un_roe = (sqrt_rhoL * unL + sqrt_rhoR * unR) / (sqrt_rhoL + sqrt_rhoR);
  const ADReal H_roe = (sqrt_rhoL * HL + sqrt_rhoR * HR) / (sqrt_rhoL + sqrt_rhoR);
  const ADReal h_roe = H_roe - 0.5 * un_roe * un_roe;
  const ADReal rho_roe = std::sqrt(rhoL * rhoR);
  const ADReal v_roe = 1.0 / rho_roe;
  const ADReal e_roe = _fp.e_from_v_h(v_roe, h_roe);
  const ADReal c_roe = _fp.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  const ADReal sL = std::min(unL - cL, un_roe - c_roe);
  const ADReal sR = std::max(unR + cR, un_roe + c_roe);
  const ADReal sm = (rhoR * unR * (sR - unR) - rhoL * unL * (sL - unL) + pL - pR) /
                    (rhoR * (sR - unR) - rhoL * (sL - unL));

  // compute Omega_L, Omega_R
  const ADReal omegL = 1.0 / (sL - sm);
  const ADReal omegR = 1.0 / (sR - sm);

  // compute p^*
  const ADReal ps = rhoL * (sL - unL) * (sm - unL) + pL;

  // compute U_L^*, U_R^*

  const ADReal rhoLs = omegL * (sL - unL) * rhoL;
  const ADReal rhouLs = omegL * ((sL - unL) * rhouL + (ps - pL) * nx);
  const ADReal rhoELs = omegL * ((sL - unL) * rhoEL - pL * unL + ps * sm);

  const ADReal rhoRs = omegR * (sR - unR) * rhoR;
  const ADReal rhouRs = omegR * ((sR - unR) * rhouR + (ps - pR) * nx);
  const ADReal rhoERs = omegR * ((sR - unR) * rhoER - pR * unR + ps * sm);

  const ADReal A_flow = computeFlowArea(UL, UR);

  // compute the fluxes
  FL.resize(THM3Eqn::N_EQ);
  if (sL > 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = uL * rhoL * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (uL * rhouL + pL) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = uL * (rhoEL + pL) * A_flow;

    _last_region_index = 0;
  }
  else if (sL <= 0.0 && sm > 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = sm * nx * rhoLs * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (sm * nx * rhouLs + ps) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = sm * nx * (rhoELs + ps) * A_flow;

    _last_region_index = 1;
  }
  else if (sm <= 0.0 && sR >= 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = sm * nx * rhoRs * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (sm * nx * rhouRs + ps) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = sm * nx * (rhoERs + ps) * A_flow;

    _last_region_index = 2;
  }
  else if (sR < 0.0)
  {
    FL[THM3Eqn::EQ_MASS] = uR * rhoR * A_flow;
    FL[THM3Eqn::EQ_MOMENTUM] = (uR * rhouR + pR) * A_flow;
    FL[THM3Eqn::EQ_ENERGY] = uR * (rhoER + pR) * A_flow;

    _last_region_index = 3;
  }
  else
  {
    FL = {getNaN(), getNaN(), getNaN()};
  }

  FR = FL;

  const ADReal A_wall_L = AL - A_flow;
  FL[THM3Eqn::EQ_MOMENTUM] += pL * A_wall_L;

  const ADReal A_wall_R = AR - A_flow;
  FR[THM3Eqn::EQ_MOMENTUM] += pR * A_wall_R;
}

ADReal
ADNumericalFlux3EqnHLLC::computeFlowArea(const std::vector<ADReal> & UL,
                                         const std::vector<ADReal> & UR) const
{
  return std::min(UL[THM3Eqn::CONS_VAR_AREA], UR[THM3Eqn::CONS_VAR_AREA]);
}
