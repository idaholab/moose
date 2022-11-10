/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "PBSodiumFluidPropertiesTest.h"

TEST_F(PBSodiumFluidPropertiesTest, density)
{
  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  ABS_TEST(_fp->rho_from_p_T(p, T), 779.284, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

TEST_F(PBSodiumFluidPropertiesTest, specificEnthalpy)
{
  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  ABS_TEST(_fp->h_from_p_T(p, T), 1.274762047677502967e+06, REL_TOL_SAVED_VALUE);
}

TEST_F(PBSodiumFluidPropertiesTest, volumetricThermalExpansionCoefficient)
{

  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  ABS_TEST(_fp->beta_from_p_T(p, T), 0.000310107566652557, REL_TOL_SAVED_VALUE);
}

TEST_F(PBSodiumFluidPropertiesTest, isobaricSpecificHeat)
{
  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  REL_TEST(_fp->cp_from_p_T(p, T), 1263.22170053911, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

TEST_F(PBSodiumFluidPropertiesTest, isochoricSpecificHeat)
{
  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  REL_TEST(_fp->cv_from_p_T(p, T), 1263.22170053911, REL_TOL_SAVED_VALUE);
}

TEST_F(PBSodiumFluidPropertiesTest, viscosity)
{
  {
    const Real T = 1000.0;
    const Real p = 2.0 * 101325;

    REL_TEST(_fp->mu_from_p_T(p, T), 0.0001858273, REL_TOL_SAVED_VALUE);
  }

  {
    const Real rho = 779.284;
    const Real T = 1000.0;

    REL_TEST(_fp->mu_from_rho_T(rho, T), 0.0001858273, REL_TOL_SAVED_VALUE);
  }
}

TEST_F(PBSodiumFluidPropertiesTest, thermalConductivity)
{
  const Real T = 1000.0;
  const Real p = 2.0 * 101325;

  REL_TEST(_fp->k_from_p_T(p, T), 58.3063, REL_TOL_SAVED_VALUE);
}

TEST_F(PBSodiumFluidPropertiesTest, temperature)
{
  const Real h = 1.0E+6;
  const Real p = 2.0 * 101325;

  REL_TEST(_fp->T_from_p_h(p, h), 782.535216773276, REL_TOL_SAVED_VALUE);
}
