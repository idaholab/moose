/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef UTILS_H
#define UTILS_H

#define REL_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs(((value) - (ref_value)) / (ref_value)), (tol)) << "    - failed " << name     \
                                                                    << " check"

#define ABS_TEST(name, value, ref_value, tol)                                                      \
  EXPECT_LE(std::abs(((value) - (ref_value))), (tol)) << "    - failed " << name << " check"

#endif // UTILS_H
