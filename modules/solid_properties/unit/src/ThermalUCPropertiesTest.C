#include "SolidPropertiesTestUtils.h"
#include "ThermalUCPropertiesTest.h"

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalUCPropertiesTest, k)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->k_from_T(T), 21.100522, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->k_from_T(T), 21.1959, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->k_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat capacity and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalUCPropertiesTest, cp)
{
  Real T;

  T = 800.0;
  REL_TEST(_sp1->cp_from_T(T), 241.5, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);

  T = 500.0;
  REL_TEST(_sp1->cp_from_T(T), 227.6, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->cp_from_T, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the density and its derivatives are
 * correctly computed.
 */
TEST_F(ThermalUCPropertiesTest, rho)
{
  Real T = 800.0;

  REL_TEST(_sp1->rho_from_T(T), 13824.7, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp1->rho_from_T, T, REL_TOL_DERIVATIVE);

  REL_TEST(_sp2->rho_from_T(T), 13000.0, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_sp2->rho_from_T, T, REL_TOL_DERIVATIVE);
}
