//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnCentered.h"
#include "THMIndices3Eqn.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADNumericalFlux3EqnCentered);

InputParameters
ADNumericalFlux3EqnCentered::validParams()
{
  InputParameters params = ADNumericalFlux3EqnBase::validParams();

  params.addClassDescription(
      "Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using a "
      "centered average of the left and right side fluxes");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

ADNumericalFlux3EqnCentered::ADNumericalFlux3EqnCentered(const InputParameters & parameters)
  : ADNumericalFlux3EqnBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADNumericalFlux3EqnCentered::calcFlux(const std::vector<ADReal> & U1,
                                      const std::vector<ADReal> & U2,
                                      const ADReal & /*nLR_dot_d*/,
                                      std::vector<ADReal> & FL,
                                      std::vector<ADReal> & FR) const
{
  const std::vector<ADReal> flux1 = computeFlux(U1);
  const std::vector<ADReal> flux2 = computeFlux(U2);

  FL.resize(THM3Eqn::N_EQ);
  for (unsigned int i = 0; i < THM3Eqn::N_EQ; i++)
    FL[i] = 0.5 * (flux1[i] + flux2[i]);

  FR = FL;
}

std::vector<ADReal>
ADNumericalFlux3EqnCentered::computeFlux(const std::vector<ADReal> & U) const
{
  const ADReal rhoA = U[THM3Eqn::CONS_VAR_RHOA];
  const ADReal rhouA = U[THM3Eqn::CONS_VAR_RHOUA];
  const ADReal rhoEA = U[THM3Eqn::CONS_VAR_RHOEA];
  const ADReal A = U[THM3Eqn::CONS_VAR_AREA];

  const ADReal rho = rhoA / A;
  const ADReal vel = rhouA / rhoA;
  const ADReal v = 1.0 / rho;
  const ADReal E = rhoEA / rhoA;
  const ADReal e = E - 0.5 * vel * vel;
  const ADReal p = _fp.p_from_v_e(v, e);
  const ADReal H = E + p / rho;

  std::vector<ADReal> flux(THM3Eqn::N_EQ, 0.0);
  flux[THM3Eqn::EQ_MASS] = rhouA;
  flux[THM3Eqn::EQ_MOMENTUM] = (rho * vel * vel + p) * A;
  flux[THM3Eqn::EQ_ENERGY] = rho * vel * H * A;

  return flux;
}
