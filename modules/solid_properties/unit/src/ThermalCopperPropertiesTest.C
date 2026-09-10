//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ThermalCopperPropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed for default RRR = 100.
 * Golden values calculated from NIST correlations.
 */
TEST_F(ThermalCopperPropertiesTest, k_rrr100)
{
  Real T;

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->k_from_T(T), 547.1997, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);

  // Test near room temperature
  T = 293.0;
  REL_TEST(_sp->k_from_T(T), 396.9085, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed. Golden values calculated from NIST correlations.
 */
TEST_F(ThermalCopperPropertiesTest, cp)
{
  Real T;

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->cp_from_T(T), 195.9209, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);

  // Test near room temperature
  T = 293.0;
  REL_TEST(_sp->cp_from_T(T), 389.0857, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific internal energy and its derivatives are
 * correctly computed. Golden value calculated by integrating NIST cp correlation.
 */
TEST_F(ThermalCopperPropertiesTest, e)
{
  const Real T = 100.0;
  REL_TEST(_sp->e_from_T(T), 10611.4135, REL_TOL_SAVED_VALUE);
  SPECIFIC_INTERNAL_ENERGY_TESTS(_sp, T, 1e-6, 1e-6);
}

/**
 * Test that the density and its derivatives are
 * correctly computed (constant density from ASM Handbook).
 */
TEST_F(ThermalCopperPropertiesTest, rho)
{
  Real T;

  T = 10.0;
  REL_TEST(_sp->rho_from_T(T), 8940.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);

  T = 293.0;
  REL_TEST(_sp->rho_from_T(T), 8940.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->rho_from_T, T, REL_TOL_DERIVATIVE);
}
