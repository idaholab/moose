//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEPHASEFLUIDPROPERTIESPTTESTUTILS_H
#define SINGLEPHASEFLUIDPROPERTIESPTTESTUTILS_H

#include "FluidPropertiesTestUtils.h"
#include "SinglePhaseFluidPropertiesPT.h"

// Macro for performing a derivative test
#define DERIV_TEST(f, fd, a, b, tol)                                                               \
  {                                                                                                \
    const Real epsilon = 1.0e-6;                                                                   \
    const Real da = epsilon * a;                                                                   \
    const Real db = epsilon * b;                                                                   \
    const Real df_da_fd = (f(a + da, b) - f(a - da, b)) / (2.0 * da);                              \
    const Real df_db_fd = (f(a, b + db) - f(a, b - db)) / (2.0 * db);                              \
                                                                                                   \
    Real f_value, df_da, df_db;                                                                    \
    fd(a, b, f_value, df_da, df_db);                                                               \
                                                                                                   \
    REL_TEST(f(a, b), f_value, REL_TOL_CONSISTENCY);                                               \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
    REL_TEST(df_db, df_db_fd, tol);                                                                \
  }

// Test methods that return multiple properties
template <typename U>
void
combinedProperties(const U & f, Real p, Real T, Real tol)
{
  // Single property methods
  Real rho, drho_dp, drho_dT;
  f->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  f->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  f->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  f->rho_mu(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  f->rho_mu_dpT(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  f->rho_e_dpT(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}

#endif // SINGLEPHASEFLUIDPROPERTIESPTTESTUTILS_H
