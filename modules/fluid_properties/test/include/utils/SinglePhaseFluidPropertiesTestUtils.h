//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluidPropertiesTestUtils.h"

// Relative perturbation for derivative tests
#define REL_PERTURBATION 1e-6

// Macro for performing a derivative test with a custom perturbation
#define DERIV_TEST_CUSTOM_PERTURBATION(f, a, b, tol, rel_pert)                                     \
  {                                                                                                \
    const Real da = rel_pert * a;                                                                  \
    const Real db = rel_pert * b;                                                                  \
    const Real df_da_fd = (f(a + da, b) - f(a - da, b)) / (2 * da);                                \
    const Real df_db_fd = (f(a, b + db) - f(a, b - db)) / (2 * db);                                \
    Real f_value, df_da, df_db;                                                                    \
    f(a, b, f_value, df_da, df_db);                                                                \
    REL_TEST(f(a, b), f_value, REL_TOL_CONSISTENCY);                                               \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
    REL_TEST(df_db, df_db_fd, tol);                                                                \
  }

#define AD_DERIV_TEST_CUSTOM_PERTURBATION(f, a, b, tol, rel_pert)                                  \
  {                                                                                                \
    const ADReal da = rel_pert * a;                                                                \
    const ADReal db = rel_pert * b;                                                                \
    const ADReal df_da_fd = (f(a + da, b) - f(a - da, b)) / (2 * da);                              \
    const ADReal df_db_fd = (f(a, b + db) - f(a, b - db)) / (2 * db);                              \
    ADReal f_value, df_da, df_db;                                                                  \
    f(a, b, f_value, df_da, df_db);                                                                \
    REL_TEST(f(a, b), f_value, REL_TOL_CONSISTENCY);                                               \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
    REL_TEST(df_db, df_db_fd, tol);                                                                \
  }

// Macro for performing a derivative test
#define DERIV_TEST(f, a, b, tol)                                                                   \
  {                                                                                                \
    DERIV_TEST_CUSTOM_PERTURBATION(f, a, b, tol, REL_PERTURBATION);                                \
  }

// Macro for performing a derivative test
#define AD_DERIV_TEST(f, a, b, tol)                                                                \
  {                                                                                                \
    AD_DERIV_TEST_CUSTOM_PERTURBATION(f, a, b, tol, REL_PERTURBATION);                             \
  }

// Macro for performing a derivative test (1d function)
#define DERIV_TEST_1D(f, dfda, a, tol)                                                             \
  {                                                                                                \
    const Real da = REL_PERTURBATION * a;                                                          \
    const Real df_da_fd = (f(a + da) - f(a - da)) / (2 * da);                                      \
    Real df_da = dfda(a);                                                                          \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
  }

// Macro for testing that a "not implemented" error message is thrown for f(a,b)
#define NOT_IMPLEMENTED_TEST_VALUE(f)                                                              \
  {                                                                                                \
    try                                                                                            \
    {                                                                                              \
      f(0, 0);                                                                                     \
    }                                                                                              \
    catch (const std::exception & x)                                                               \
    {                                                                                              \
      std::string msg(x.what());                                                                   \
      EXPECT_TRUE(msg.find("not implemented") != std::string::npos);                               \
    }                                                                                              \
  }

// Macro for testing that a "not implemented" error message is thrown for f(a,b,c,d,e)
#define NOT_IMPLEMENTED_TEST_DERIV(f)                                                              \
  {                                                                                                \
    try                                                                                            \
    {                                                                                              \
      Real f_val, df_da, df_db;                                                                    \
      f(0, 0, f_val, df_da, df_db);                                                                \
    }                                                                                              \
    catch (const std::exception & x)                                                               \
    {                                                                                              \
      std::string msg(x.what());                                                                   \
      EXPECT_TRUE(msg.find("not implemented") != std::string::npos);                               \
    }                                                                                              \
  }
