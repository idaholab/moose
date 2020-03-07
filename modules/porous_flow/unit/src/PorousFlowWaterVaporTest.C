//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowWaterVaporTest.h"
#include "FluidPropertiesTestUtils.h"

/**
 * Verify that the correct name is supplied
 */
TEST_F(PorousFlowWaterVaporTest, name) { EXPECT_EQ("water-vapor", _fp->fluidStateName()); }

TEST_F(PorousFlowWaterVaporTest, properties)
{
  FluidStatePhaseEnum phase_state;
  std::vector<FluidStateProperties> fsp(2, FluidStateProperties(1));

  // Single phase liquid region
  Real p = 1.0e6;
  Real h = 1.0e5;

  const Real tol = REL_TOL_CONSISTENCY;

  _fp->thermophysicalProperties(p, h, 0, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::LIQUID);

  Real T = _water_fp->T_from_p_h(p, h);

  ABS_TEST(fsp[0].density.value(), _water_fp->rho_from_p_T(p, T), tol);
  ABS_TEST(fsp[0].viscosity.value(), _water_fp->mu_from_p_T(p, T), tol);
  ABS_TEST(fsp[0].enthalpy.value(), h, tol);
  ABS_TEST(fsp[0].internal_energy.value(), _water_fp->e_from_p_T(p, T), tol);
  ABS_TEST(fsp[0].saturation.value(), 1.0, tol);
  ABS_TEST(fsp[0].pressure.value(), p, tol);

  // Single phase gas region
  h = 3.0e6;
  Real pg = p + _pc->capillaryPressure(0.0);

  _fp->thermophysicalProperties(p, h, 0, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::GAS);

  T = _water_fp->T_from_p_h(pg, h);
  ABS_TEST(fsp[1].density.value(), _water_fp->rho_from_p_T(pg, T), tol);
  ABS_TEST(fsp[1].viscosity.value(), _water_fp->mu_from_p_T(pg, T), tol);
  ABS_TEST(fsp[1].enthalpy.value(), h, tol);
  ABS_TEST(fsp[1].internal_energy.value(), _water_fp->e_from_p_T(pg, T), tol);
  ABS_TEST(fsp[1].saturation.value(), 1.0, tol);
  ABS_TEST(fsp[1].pressure.value(), pg, tol);

  // Two-phase region
  h = 8.0e5;

  _fp->thermophysicalProperties(p, h, 0, phase_state, fsp);
  EXPECT_EQ(phase_state, FluidStatePhaseEnum::TWOPHASE);

  const Real Tsat = _water_fp->vaporTemperature(p);
  const Real dT = 1.0e-6;

  // Liquid phase
  ABS_TEST(fsp[0].density.value(), _water_fp->rho_from_p_T(p, Tsat - dT), tol);
  ABS_TEST(fsp[0].viscosity.value(), _water_fp->mu_from_p_T(p, Tsat - dT), tol);
  ABS_TEST(fsp[0].enthalpy.value(), _water_fp->h_from_p_T(p, Tsat - dT), tol);
  ABS_TEST(fsp[0].internal_energy.value(), _water_fp->e_from_p_T(p, Tsat - dT), tol);
  ABS_TEST(fsp[0].pressure.value(), p, tol);

  // Gas phase
  pg = p + _pc->capillaryPressure(fsp[0].saturation.value());
  ABS_TEST(fsp[1].density.value(), _water_fp->rho_from_p_T(p, Tsat + dT), tol);
  ABS_TEST(fsp[1].viscosity.value(), _water_fp->mu_from_p_T(p, Tsat + dT), tol);
  ABS_TEST(fsp[1].enthalpy.value(), _water_fp->h_from_p_T(p, Tsat + dT), tol);
  ABS_TEST(fsp[1].internal_energy.value(), _water_fp->e_from_p_T(p, Tsat + dT), tol);
  ABS_TEST(fsp[1].pressure.value(), pg, tol);
}
