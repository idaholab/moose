//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesTestUtils.h"
#include "ElectromagneticCryogenicJK2LBPropertiesTest.h"

/**
 * Test that the electrical resistivity and its derivatives are
 * correctly computed. Golden values calculated from quintic polynomial correlation.
 */
TEST_F(ElectromagneticCryogenicJK2LBPropertiesTest, electrical_resistivity)
{
  Real T;

  // Test at minimum temperature
  T = 2.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 9.12016252726776708e-07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 9.13019389415845862e-07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 9.30207191511528217e-07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at Neel temperature (antiferromagnetic transition)
  T = 240.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 9.49513713189960750e-07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 300.0;
  REL_TEST(_sp->electrical_resistivity_from_T(T), 9.90397363189634310e-07, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_resistivity_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the electrical conductivity and its derivatives are
 * correctly computed. Golden values calculated as reciprocal of resistivity.
 */
TEST_F(ElectromagneticCryogenicJK2LBPropertiesTest, electrical_conductivity)
{
  Real T;

  // Test at minimum temperature
  T = 2.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 1.09647168787855096e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature (critical for ITER cryogenics)
  T = 77.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 1.09526699168985314e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at mid-range temperature
  T = 150.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 1.07502931510888762e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at Neel temperature (antiferromagnetic transition)
  T = 240.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 1.05317067685144534e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 300.0;
  REL_TEST(_sp->electrical_conductivity_from_T(T), 1.00969574149454501e+06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->electrical_conductivity_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the magnetic permeability and its derivatives are
 * correctly computed. Golden value is constant mu_0 (austenitic steel is non-magnetic).
 */
TEST_F(ElectromagneticCryogenicJK2LBPropertiesTest, magnetic_permeability)
{
  Real T;

  // Test at low temperature
  T = 2.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706211999994e-06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);

  // Test at liquid nitrogen temperature
  T = 77.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706211999994e-06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);

  // Test at room temperature
  T = 300.0;
  REL_TEST(_sp->magnetic_permeability_from_T(T), 1.25663706211999994e-06, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp->magnetic_permeability_from_T, T, REL_TOL_DERIVATIVE);
}
