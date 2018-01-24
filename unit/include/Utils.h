//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef UTILS_H
#define UTILS_H

#define REL_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs(((value) - (ref_value)) / (ref_value)), (tol)) << "    - failed " << name     \
                                                                    << " check"

#define ABS_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs(((value) - (ref_value))), (tol)) << "    - failed " << name << " check"

#endif // UTILS_H
