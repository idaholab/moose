//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFluxGasMixHLLC.h"
#include "VaporMixtureFluidProperties.h"
#include "THMIndicesGasMix.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", NumericalFluxGasMixHLLC);

InputParameters
NumericalFluxGasMixHLLC::validParams()
{
  InputParameters params = NumericalFluxGasMixBase::validParams();
  params += NaNInterface::validParams();
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");
  params.addClassDescription("Computes internal side flux for the 1-D, 1-phase, variable-area "
                             "Euler equations using the HLLC approximate Riemann solver.");
  return params;
}

NumericalFluxGasMixHLLC::NumericalFluxGasMixHLLC(const InputParameters & parameters)
  : NumericalFluxGasMixBase(parameters),
    NaNInterface(this),
    _fp(getUserObject<VaporMixtureFluidProperties>("fluid_properties"))
{
}

void
NumericalFluxGasMixHLLC::calcFlux(const std::vector<ADReal> & UL,
                                  const std::vector<ADReal> & UR,
                                  const RealVectorValue & nLR,
                                  const RealVectorValue & t1,
                                  const RealVectorValue & t2,
                                  std::vector<ADReal> & FL,
                                  std::vector<ADReal> & FR) const
{
  // extract the conserved variables and area

  const ADReal xirhoAL = UL[THMGasMix3D::XIRHOA];
  const ADReal rhoAL = UL[THMGasMix3D::RHOA];
  const ADReal rhouAL = UL[THMGasMix3D::RHOUA];
  const ADReal rhovAL = UL[THMGasMix3D::RHOVA];
  const ADReal rhowAL = UL[THMGasMix3D::RHOWA];
  const ADReal rhoEAL = UL[THMGasMix3D::RHOEA];
  const ADReal AL = UL[THMGasMix3D::AREA];

  const ADReal xirhoAR = UR[THMGasMix3D::XIRHOA];
  const ADReal rhoAR = UR[THMGasMix3D::RHOA];
  const ADReal rhouAR = UR[THMGasMix3D::RHOUA];
  const ADReal rhovAR = UR[THMGasMix3D::RHOVA];
  const ADReal rhowAR = UR[THMGasMix3D::RHOWA];
  const ADReal rhoEAR = UR[THMGasMix3D::RHOEA];
  const ADReal AR = UR[THMGasMix3D::AREA];

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
  const ADReal xiL = xirhoAL / rhoAL;
  const ADReal pL = _fp.p_from_v_e(vL, eL, {xiL});
  const ADReal TL = _fp.T_from_v_e(vL, eL, {xiL});
  const ADReal cL = _fp.c_from_p_T(pL, TL, {xiL});

  const ADReal rhoR = rhoAR / AR;
  const ADRealVectorValue uvecR(rhouAR / rhoAR, rhovAR / rhoAR, rhowAR / rhoAR);
  const ADReal unR = uvecR * nLR;
  const ADReal ut1R = uvecR * t1;
  const ADReal ut2R = uvecR * t2;
  const ADReal rhoER = rhoEAR / AR;
  const ADReal vR = 1.0 / rhoR;
  const ADReal ER = rhoEAR / rhoAR;
  const ADReal eR = ER - 0.5 * uvecR * uvecR;
  const ADReal xiR = xirhoAR / rhoAR;
  const ADReal pR = _fp.p_from_v_e(vR, eR, {xiR});
  const ADReal TR = _fp.T_from_v_e(vR, eR, {xiR});
  const ADReal cR = _fp.c_from_p_T(pR, TR, {xiR});

  // compute wave speeds
  const ADReal sL = std::min(unL - cL, unR - cR);
  const ADReal sR = std::max(unL + cL, unR + cR);
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

  std::vector<ADReal> UL_1d(THMGasMix1D::N_FLUX_INPUTS);
  UL_1d[THMGasMix1D::XIRHOA] = UL[THMGasMix3D::XIRHOA];
  UL_1d[THMGasMix1D::RHOA] = UL[THMGasMix3D::RHOA];
  UL_1d[THMGasMix1D::RHOUA] = UL[THMGasMix3D::RHOUA];
  UL_1d[THMGasMix1D::RHOEA] = UL[THMGasMix3D::RHOEA];
  UL_1d[THMGasMix1D::AREA] = UL[THMGasMix3D::AREA];

  std::vector<ADReal> UR_1d(THMGasMix1D::N_FLUX_INPUTS);
  UR_1d[THMGasMix1D::XIRHOA] = UR[THMGasMix3D::XIRHOA];
  UR_1d[THMGasMix1D::RHOA] = UR[THMGasMix3D::RHOA];
  UR_1d[THMGasMix1D::RHOUA] = UR[THMGasMix3D::RHOUA];
  UR_1d[THMGasMix1D::RHOEA] = UR[THMGasMix3D::RHOEA];
  UR_1d[THMGasMix1D::AREA] = UR[THMGasMix3D::AREA];

  const ADReal A_flow = computeFlowArea(UL_1d, UR_1d);

  // compute the fluxes
  FL.resize(THMGasMix3D::N_FLUX_OUTPUTS);
  if (sL > 0.0)
  {
    FL[THMGasMix3D::SPECIES] = unL * xiL * rhoL * A_flow;
    FL[THMGasMix3D::MASS] = unL * rhoL * A_flow;
    FL[THMGasMix3D::MOM_NORM] = (unL * rhoL * unL + pL) * A_flow;
    FL[THMGasMix3D::MOM_TAN1] = rhoL * unL * ut1L * A_flow;
    FL[THMGasMix3D::MOM_TAN2] = rhoL * unL * ut2L * A_flow;
    FL[THMGasMix3D::ENERGY] = unL * (rhoEL + pL) * A_flow;

    _last_region_index = 0;
  }
  else if (sL <= 0.0 && sm > 0.0)
  {
    FL[THMGasMix3D::SPECIES] = sm * xiL * rhoLs * A_flow;
    FL[THMGasMix3D::MASS] = sm * rhoLs * A_flow;
    FL[THMGasMix3D::MOM_NORM] = (sm * rhounLs + ps) * A_flow;
    FL[THMGasMix3D::MOM_TAN1] = rhounLs * ut1L * A_flow;
    FL[THMGasMix3D::MOM_TAN2] = rhounLs * ut2L * A_flow;
    FL[THMGasMix3D::ENERGY] = sm * (rhoELs + ps) * A_flow;

    _last_region_index = 1;
  }
  else if (sm <= 0.0 && sR >= 0.0)
  {
    FL[THMGasMix3D::SPECIES] = sm * xiR * rhoRs * A_flow;
    FL[THMGasMix3D::MASS] = sm * rhoRs * A_flow;
    FL[THMGasMix3D::MOM_NORM] = (sm * rhounRs + ps) * A_flow;
    FL[THMGasMix3D::MOM_TAN1] = rhounRs * ut1R * A_flow;
    FL[THMGasMix3D::MOM_TAN2] = rhounRs * ut2R * A_flow;
    FL[THMGasMix3D::ENERGY] = sm * (rhoERs + ps) * A_flow;

    _last_region_index = 2;
  }
  else if (sR < 0.0)
  {
    FL[THMGasMix3D::SPECIES] = unR * xiR * rhoR * A_flow;
    FL[THMGasMix3D::MASS] = unR * rhoR * A_flow;
    FL[THMGasMix3D::MOM_NORM] = (unR * rhoR * unR + pR) * A_flow;
    FL[THMGasMix3D::MOM_TAN1] = rhoR * unR * ut1R * A_flow;
    FL[THMGasMix3D::MOM_TAN2] = rhoR * unR * ut2R * A_flow;
    FL[THMGasMix3D::ENERGY] = unR * (rhoER + pR) * A_flow;

    _last_region_index = 3;
  }
  else
    std::fill(FL.begin(), FL.end(), getNaN());

  FR = FL;

  const ADReal A_wall_L = AL - A_flow;
  FL[THMGasMix3D::MOM_NORM] += pL * A_wall_L;

  const ADReal A_wall_R = AR - A_flow;
  FR[THMGasMix3D::MOM_NORM] += pR * A_wall_R;
}

ADReal
NumericalFluxGasMixHLLC::computeFlowArea(const std::vector<ADReal> & UL,
                                         const std::vector<ADReal> & UR) const
{
  return std::min(UL[THMGasMix1D::AREA], UR[THMGasMix1D::AREA]);
}
