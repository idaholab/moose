#include "StiffenedGas7EqnFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

TEST_F(StiffenedGas7EqnFluidPropertiesTest, test)
{
  {
    Real p = 101325; // Pa

    /*
     * The following reference gives the saturation state to have equal Gibbs
     * free enthalpy between the 2 phases.
     *
     * Ray A. Berry, Richard Saurel, Olivier LeMetayer
     * The discrete equation method (DEM) for fully compressible, two-phase flow
     *   in ducts of spatially varying cross-section
     * Nuclear Engineering and Design 240 (2010) p. 3797-3818
     *
     * Therefore, it should be asserted that the nonlinear solver for the saturation
     * temperature returns a state for which this is true.
     */
    const Real T_sat = _fp->T_sat(p);
    // liquid
    const Real rho_liquid = _fp_liquid->rho_from_p_T(p, T_sat);
    const Real v_liquid = 1.0 / rho_liquid;
    const Real e_liquid = _fp_liquid->e_from_p_rho(p, rho_liquid);
    const Real g_liquid = _fp_liquid->g_from_v_e(v_liquid, e_liquid);
    // vapor
    const Real rho_vapor = _fp_vapor->rho_from_p_T(p, T_sat);
    const Real v_vapor = 1.0 / rho_vapor;
    const Real e_vapor = _fp_vapor->e_from_p_rho(p, rho_vapor);
    const Real g_vapor = _fp_vapor->g_from_v_e(v_vapor, e_vapor);

    REL_TEST(g_liquid, g_vapor, 1e-5);

    // ensure that p_sat(T_sat(p)) is the inverse of T_sat(p)
    REL_TEST(_fp->p_sat(T_sat), p, 1e-14);

    // test derivative along saturation curve using FD approximation
    Real dT_dPsat = _fp->dT_sat_dp(p);
    Real dp = 1e1;
    Real dT_dPsat_fd = (_fp->T_sat(p + dp) - _fp->T_sat(p - dp)) / (2 * dp);

    EXPECT_NEAR(dT_dPsat, dT_dPsat_fd, 1e-14);
  }

  EXPECT_NEAR(_fp->p_critical(), 22.09e6, 1e-15);
}
