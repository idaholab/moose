//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "metaphysicl/raw_type.h"

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

// Macro for computing relative error
#define REL_TEST(value, ref_value, tol)                                                            \
  if (std::abs(ref_value) < 1e-15)                                                                 \
    ABS_TEST(value, ref_value, tol);                                                               \
  else                                                                                             \
    EXPECT_LE(std::abs(((MetaPhysicL::raw_value(value)) - (MetaPhysicL::raw_value(ref_value))) /   \
                       (MetaPhysicL::raw_value(ref_value))),                                       \
              tol);

// Macro for computing absolute error
#define ABS_TEST(value, ref_value, tol)                                                            \
  EXPECT_LE(std::abs(((MetaPhysicL::raw_value(value)) - (MetaPhysicL::raw_value(ref_value)))),     \
            (tol))
