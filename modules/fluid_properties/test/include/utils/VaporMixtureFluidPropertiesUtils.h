//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidPropertiesTestUtils.h"

// Macro for performing a derivative test
#define VAPOR_MIX_DERIV_TEST(f, a, b, x, tol)                                                      \
  {                                                                                                \
    const Real da = REL_PERTURBATION * a;                                                          \
    const Real db = REL_PERTURBATION * b;                                                          \
    std::vector<Real> dx(x.size());                                                                \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      dx[i] = REL_PERTURBATION * x[i];                                                             \
    const Real df_da_fd = (f(a + da, b, x) - f(a - da, b, x)) / (2 * da);                          \
    const Real df_db_fd = (f(a, b + db, x) - f(a, b - db, x)) / (2 * db);                          \
    std::vector<Real> df_dx_fd(x.size());                                                          \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
    {                                                                                              \
      std::vector<Real> x_forward(x);                                                              \
      x_forward[i] += dx[i];                                                                       \
      std::vector<Real> x_backward(x);                                                             \
      x_backward[i] -= dx[i];                                                                      \
      df_dx_fd[i] = (f(a, b, x_forward) - f(a, b, x_backward)) / (2 * dx[i]);                      \
    }                                                                                              \
    Real f_value, df_da, df_db;                                                                    \
    std::vector<Real> df_dx(x.size());                                                             \
    f(a, b, x, f_value, df_da, df_db, df_dx);                                                      \
    REL_TEST(f(a, b, x), f_value, REL_TOL_CONSISTENCY);                                            \
    REL_TEST(df_da, df_da_fd, tol);                                                                \
    REL_TEST(df_db, df_db_fd, tol);                                                                \
    for (unsigned int i = 0; i < x.size(); ++i)                                                    \
      REL_TEST(df_dx[i], df_dx_fd[i], tol);                                                        \
  }
