//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Relative tolerance to be used when comparing to a value from external fluid
// property packages, which might be using different underlying models
#define REL_TOL_EXTERNAL_VALUE 1e-3

// Relative tolerance to be used when comparing to a value computed directly
// from the code at an earlier date, to ensure that implementations are not
// inadvertently changed
#define REL_TOL_SAVED_VALUE 1e-12

// Relative tolerance to be used for consistency checks - computing properties
// in different ways at the same state
#define REL_TOL_CONSISTENCY 1e-10

// Relative tolerance to be used when comparing a derivative value to a finite
// difference approximation
#define REL_TOL_DERIVATIVE 1e-6

// Relative perturbation for derivative tests
#define REL_PERTURBATION 1e-6

// Macro for computing relative error
#define REL_TEST(value, ref_value, tol)                                                            \
  if (std::abs(ref_value) < 1e-15)                                                                 \
    ABS_TEST(value, ref_value, tol);                                                               \
  else                                                                                             \
    EXPECT_LE(std::abs(((value) - (ref_value)) / (ref_value)), tol);

// Macro for computing absolute error
#define ABS_TEST(value, ref_value, tol) EXPECT_LE(std::abs(((value) - (ref_value))), (tol))

// Macro for performing a derivative test with a custom perturbation for 1 parameter
#define DERIV_TEST_CUSTOM_PERTURBATION(f, a, tol, rel_pert)                                        \
  {                                                                                                \
    const Real da = rel_pert * a;                                                                  \
    const Real df_da_fd = (f(a + da) - f(a - da)) / (2 * da);                                      \
    Real f_value, df_da;                                                                           \
    f(a, f_value, df_da);                                                                          \
    REL_TEST(f(a), f_value, REL_TOL_CONSISTENCY);                                                  \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
  }

// Macro for performing a derivative test with 1 parameter
#define DERIV_TEST(f, a, tol)                                                                      \
  {                                                                                                \
    DERIV_TEST_CUSTOM_PERTURBATION(f, a, tol, REL_PERTURBATION);                                   \
  }
