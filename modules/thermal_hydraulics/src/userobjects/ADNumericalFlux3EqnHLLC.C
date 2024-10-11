//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnHLLC.h"
#include "THMIndicesVACE.h"
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
                                  const RealVectorValue & nLR,
                                  const RealVectorValue & t1,
                                  const RealVectorValue & t2,
                                  std::vector<ADReal> & FL,
                                  std::vector<ADReal> & FR) const
{
  // extract the conserved variables and area

  const ADReal rhoAL = UL[THMVACE3D::RHOA];
  const ADReal rhouAL = UL[THMVACE3D::RHOUA];
  const ADReal rhovAL = UL[THMVACE3D::RHOVA];
  const ADReal rhowAL = UL[THMVACE3D::RHOWA];
  const ADReal rhoEAL = UL[THMVACE3D::RHOEA];
  const ADReal AL = UL[THMVACE3D::AREA];

  const ADReal rhoAR = UR[THMVACE3D::RHOA];
  const ADReal rhouAR = UR[THMVACE3D::RHOUA];
  const ADReal rhovAR = UR[THMVACE3D::RHOVA];
  const ADReal rhowAR = UR[THMVACE3D::RHOWA];
  const ADReal rhoEAR = UR[THMVACE3D::RHOEA];
  const ADReal AR = UR[THMVACE3D::AREA];

  // compute the primitive variables

  const ADReal rhoL = rhoAL / AL;
  const ADRealVectorValue uvecL(rhouAL / rhoAL, rhovAL / rhoAL, rhowAL / rhoAL);
  const ADReal unL = uvecL * nLR;
  const ADReal ut1L = uvecL * t1;
  const ADReal ut2L = uvecL * t2;
  const ADReal rhoEL = rhoEAL / AL;
  const ADReal vL = 1.0 / rhoL;
  const ADReal EL = rhoEAL / rhoAL;
  const ADReal eL = EL - 0.5 * uvecL * uvecL;
  const ADReal pL = _fp.p_from_v_e(vL, eL);
  const ADReal HL = EL + pL / rhoL;
  const ADReal cL = _fp.c_from_v_e(vL, eL);

  const ADReal rhoR = rhoAR / AR;
  const ADRealVectorValue uvecR(rhouAR / rhoAR, rhovAR / rhoAR, rhowAR / rhoAR);
  const ADReal unR = uvecR * nLR;
  const ADReal ut1R = uvecR * t1;
  const ADReal ut2R = uvecR * t2;
  const ADReal rhoER = rhoEAR / AR;
  const ADReal vR = 1.0 / rhoR;
  const ADReal ER = rhoEAR / rhoAR;
  const ADReal eR = ER - 0.5 * uvecR * uvecR;
  const ADReal pR = _fp.p_from_v_e(vR, eR);
  const ADReal HR = ER + pR / rhoR;
  const ADReal cR = _fp.c_from_v_e(vR, eR);

  // compute Roe-averaged variables
  const ADReal sqrt_rhoL = std::sqrt(rhoL);
  const ADReal sqrt_rhoR = std::sqrt(rhoR);
  const ADReal un_roe = (sqrt_rhoL * unL + sqrt_rhoR * unR) / (sqrt_rhoL + sqrt_rhoR);
  const ADReal ut1_roe = (sqrt_rhoL * ut1L + sqrt_rhoR * ut1R) / (sqrt_rhoL + sqrt_rhoR);
  const ADReal ut2_roe = (sqrt_rhoL * ut2L + sqrt_rhoR * ut2R) / (sqrt_rhoL + sqrt_rhoR);
  const ADReal H_roe = (sqrt_rhoL * HL + sqrt_rhoR * HR) / (sqrt_rhoL + sqrt_rhoR);
  const ADRealVectorValue uvec_roe(un_roe, ut1_roe, ut2_roe);
  const ADReal h_roe = H_roe - 0.5 * uvec_roe * uvec_roe;
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
  const ADReal rhounLs = omegL * ((sL - unL) * rhoL * unL + ps - pL);
  const ADReal rhoELs = omegL * ((sL - unL) * rhoEL - pL * unL + ps * sm);

  const ADReal rhoRs = omegR * (sR - unR) * rhoR;
  const ADReal rhounRs = omegR * ((sR - unR) * rhoR * unR + ps - pR);
  const ADReal rhoERs = omegR * ((sR - unR) * rhoER - pR * unR + ps * sm);

  std::vector<ADReal> UL_1d(THMVACE1D::N_FLUX_INPUTS);
  UL_1d[THMVACE1D::RHOA] = UL[THMVACE3D::RHOA];
  UL_1d[THMVACE1D::RHOUA] = UL[THMVACE3D::RHOUA];
  UL_1d[THMVACE1D::RHOEA] = UL[THMVACE3D::RHOEA];
  UL_1d[THMVACE1D::AREA] = UL[THMVACE3D::AREA];

  std::vector<ADReal> UR_1d(THMVACE1D::N_FLUX_INPUTS);
  UR_1d[THMVACE1D::RHOA] = UR[THMVACE3D::RHOA];
  UR_1d[THMVACE1D::RHOUA] = UR[THMVACE3D::RHOUA];
  UR_1d[THMVACE1D::RHOEA] = UR[THMVACE3D::RHOEA];
  UR_1d[THMVACE1D::AREA] = UR[THMVACE3D::AREA];

  const ADReal A_flow = computeFlowArea(UL_1d, UR_1d);

  // compute the fluxes
  FL.resize(THMVACE3D::N_FLUX_OUTPUTS);
  if (sL > 0.0)
  {
    FL[THMVACE3D::MASS] = unL * rhoL * A_flow;
    FL[THMVACE3D::MOM_NORM] = (unL * rhoL * unL + pL) * A_flow;
    FL[THMVACE3D::MOM_TAN1] = rhoL * unL * ut1L * A_flow;
    FL[THMVACE3D::MOM_TAN2] = rhoL * unL * ut2L * A_flow;
    FL[THMVACE3D::ENERGY] = unL * (rhoEL + pL) * A_flow;

    _last_region_index = 0;
  }
  else if (sL <= 0.0 && sm > 0.0)
  {
    FL[THMVACE3D::MASS] = sm * rhoLs * A_flow;
    FL[THMVACE3D::MOM_NORM] = (sm * rhounLs + ps) * A_flow;
    FL[THMVACE3D::MOM_TAN1] = rhounLs * ut1L * A_flow;
    FL[THMVACE3D::MOM_TAN2] = rhounLs * ut2L * A_flow;
    FL[THMVACE3D::ENERGY] = sm * (rhoELs + ps) * A_flow;

    _last_region_index = 1;
  }
  else if (sm <= 0.0 && sR >= 0.0)
  {
    FL[THMVACE3D::MASS] = sm * rhoRs * A_flow;
    FL[THMVACE3D::MOM_NORM] = (sm * rhounRs + ps) * A_flow;
    FL[THMVACE3D::MOM_TAN1] = rhounRs * ut1R * A_flow;
    FL[THMVACE3D::MOM_TAN2] = rhounRs * ut2R * A_flow;
    FL[THMVACE3D::ENERGY] = sm * (rhoERs + ps) * A_flow;

    _last_region_index = 2;
  }
  else if (sR < 0.0)
  {
    FL[THMVACE3D::MASS] = unR * rhoR * A_flow;
    FL[THMVACE3D::MOM_NORM] = (unR * rhoR * unR + pR) * A_flow;
    FL[THMVACE3D::MOM_TAN1] = rhoR * unR * ut1R * A_flow;
    FL[THMVACE3D::MOM_TAN2] = rhoR * unR * ut2R * A_flow;
    FL[THMVACE3D::ENERGY] = unR * (rhoER + pR) * A_flow;

    _last_region_index = 3;
  }
  else
    std::fill(FL.begin(), FL.end(), getNaN());

  FR = FL;

  const ADReal A_wall_L = AL - A_flow;
  FL[THMVACE3D::MOM_NORM] += pL * A_wall_L;

  const ADReal A_wall_R = AR - A_flow;
  FR[THMVACE3D::MOM_NORM] += pR * A_wall_R;
}

ADReal
ADNumericalFlux3EqnHLLC::computeFlowArea(const std::vector<ADReal> & UL,
                                         const std::vector<ADReal> & UR) const
{
  return std::min(UL[THMVACE1D::AREA], UR[THMVACE1D::AREA]);
}
