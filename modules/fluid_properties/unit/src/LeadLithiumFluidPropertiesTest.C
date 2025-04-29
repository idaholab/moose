//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeadLithiumFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(LeadLithiumFluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "LeadLithium"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(LeadLithiumFluidPropertiesTest, molarMass)
{
  // Expected molar mass: 0.1730 kg/mol
  REL_TEST(_fp->molarMass(), 0.1730, REL_TOL_SAVED_VALUE);
}

/**
 * Verify calculation of the LeadLithium fluid properties.
 *
 * The expected reference values are computed from:
 *
 *   rho(T) = 10520.35 - 1.19051*T
 *   h(T)   = 195*(T - 508) - 0.5*9.116e-3*(T^2 - 508^2)    (T_mo = 508 K)
 *   e      = h - p/rho
 *   k(T)   = 9.144 + 0.019631*T
 *   E(T)   = (44.73077 - 0.02634615*T + 5.76923e-6*T^2)*1e9
 *   c(T)   = 1959.63 - 0.306*T
 *   cp(T)  = 195 - 9.116e-3*T
 *   cv(T)  = cp/(1 + alpha^2*E*T/(rho*cp)),  alpha  = 1.19051/(10520.35 - 1.19051*T)
 *   mu(T)  = 1.87e-4 * exp(11640/(R*T))
 *
 * The test uses three pressure/temperature pairs.
 */
TEST_F(LeadLithiumFluidPropertiesTest, properties)
{
  const Real tol = REL_TOL_SAVED_VALUE;
  const std::vector<Real> pressures = {100000.00, 1000000.00, 5000000.00};
  const std::vector<Real> temperatures = {700.0000, 800.0000, 1000.0000};

  // Nested loops over pressure and temperature
  for (size_t ip = 0; ip < pressures.size(); ip++)
  {
    const Real p = pressures[ip];
    for (size_t iT = 0; iT < temperatures.size(); iT++)
    {
      const Real T = temperatures[iT];

      // Compute reference density:
      const Real rho_ref = 10520.35 - 1.19051 * T;
      // Compute reference enthalpy (using T_mo = 508 K):
      const Real h_ref = 195.0 * (T - 508.0) - 0.5 * 9.116e-3 * (T * T - 508.0 * 508.0);
      // Compute reference internal energy:
      const Real e_ref = h_ref - p / rho_ref;
      // Compute thermal conductivity:
      const Real k_ref = 14.51 + 0.019631 * T;
      // Compute bulk modulus:
      const Real E_ref = (44.73077 - 0.02634615 * T + 5.76923e-6 * T * T) * 1e9;
      // Compute speed of sound:
      const Real c_ref = 1959.63 - 0.306 * T;
      // Compute isobaric specific heat:
      const Real cp_ref = 195.0 - 9.116e-3 * T;
      // Compute thermal expansion coefficient:
      const Real alpha = 1.19051 / (10520.35 - 1.19051 * T);
      // Compute isochoric specific heat:
      const Real cv_ref = cp_ref / (1.0 + (alpha * alpha * E_ref * T) / (rho_ref * cp_ref));
      // Compute dynamic viscosity:
      const Real mu_ref = 1.87e-4 * std::exp(11640.0 / (FluidProperties::_R * T));

      // Obtain computed values from the property object
      const Real rho = _fp->rho_from_p_T(p, T);
      const Real v = 1.0 / rho;
      const Real h = _fp->h_from_p_T(p, T);
      const Real e = _fp->e_from_p_T(p, T);

      // Test density and specific volume
      REL_TEST(rho, rho_ref, tol);
      REL_TEST(_fp->v_from_p_T(p, T), 1.0 / rho_ref, tol);

      // Test pressure from (v, e): p = (h - e)/v
      REL_TEST(_fp->p_from_v_e(v, e), p, tol * 1e5);

      // Test enthalpy (both from p,T and v,e)
      REL_TEST(h, h_ref, tol);
      REL_TEST(_fp->h_from_v_e(v, e), h_ref, tol);

      // Test internal energy (both from p,T and from p,rho)
      REL_TEST(e, e_ref, tol);
      REL_TEST(_fp->e_from_p_rho(p, rho), e_ref, tol);

      // Test temperature (from v,e, from p,, and from p,h)
      REL_TEST(_fp->T_from_v_e(v, e), T, tol);
      REL_TEST(_fp->T_from_p_rho(p, rho), T, tol);
      REL_TEST(_fp->T_from_p_h(p, h), T, tol);

      // Test thermal conductivity
      REL_TEST(_fp->k_from_p_T(p, T), k_ref, tol);
      REL_TEST(_fp->k_from_v_e(v, e), k_ref, tol);

      // Test bulk modulus
      REL_TEST(_fp->bulk_modulus_from_p_T(p, T), E_ref, tol);

      // Test speed of sound
      REL_TEST(_fp->c_from_v_e(v, e), c_ref, tol);

      // Test isobaric specific heat
      REL_TEST(_fp->cp_from_p_T(p, T), cp_ref, tol);
      REL_TEST(_fp->cp_from_v_e(v, e), cp_ref, tol);

      // Test isochoric specific heat
      REL_TEST(_fp->cv_from_p_T(p, T), cv_ref, tol);
      REL_TEST(_fp->cv_from_v_e(v, e), cv_ref, tol);

      // Test dynamic viscosity
      REL_TEST(_fp->mu_from_p_T(p, T), mu_ref, tol);
      REL_TEST(_fp->mu_from_v_e(v, e), mu_ref, tol);
    }
  }
}

/**
 * Verify the calculation of the derivatives of LeadLithium properties
 * by comparing with finite differences.
 *
 * Note: The test temperature is chosen above the melting point (e.g., 600 K)
 * to ensure that the correlations are evaluated in the valid liquid range.
 */
TEST_F(LeadLithiumFluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  const Real p = 30000000.0000;
  const Real T = 600.0000; // Temperature above 508 K
  const Real rho = _fp->rho_from_p_T(p, T);
  const Real v = 1.0 / rho;
  const Real h = _fp->h_from_p_T(p, T);
  const Real e = _fp->e_from_p_T(p, T);

  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->v_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);
  DERIV_TEST(_fp->k_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cp_from_p_T, p, T, tol);
  DERIV_TEST(_fp->cv_from_p_T, p, T, 1.5e-6);
  DERIV_TEST(_fp->mu_from_p_T, p, T, tol);

  DERIV_TEST(_fp->p_from_v_e, v, e, tol);
  DERIV_TEST(_fp->mu_from_v_e, v, e, tol);
  DERIV_TEST(_fp->k_from_v_e, v, e, tol);
  DERIV_TEST(_fp->h_from_v_e, v, e, tol);
  DERIV_TEST(_fp->T_from_v_e, v, e, tol);
  DERIV_TEST(_fp->cp_from_v_e, v, e, tol);
  DERIV_TEST(_fp->cv_from_v_e, v, e, tol);

  DERIV_TEST(_fp->T_from_p_rho, p, rho, tol);
  DERIV_TEST(_fp->e_from_p_rho, p, rho, tol);
  DERIV_TEST(_fp->T_from_p_h, p, h, tol);
}
