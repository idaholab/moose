//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "NumericalFlux1D.h"

/**
 * Base class for testing NumericalFlux1D objects
 */
class TestNumericalFlux1D : public MooseObjectUnitTest
{
public:
  TestNumericalFlux1D();

protected:
  /**
   * Creates the flux object to be tested
   */
  virtual const NumericalFlux1D & createFluxObject() = 0;

  /**
   * Computes the conservative solution from the primitive solution
   *
   * @param[in] W   Primitive solution vector
   * @param[in] A   Cross-sectional area
   */
  virtual std::vector<ADReal> computeConservativeSolution(const std::vector<ADReal> & W,
                                                  const ADReal & A) const = 0;

  /**
   * Computes the 1D flux vector from the primitive solution
   *
   * @param[in] W   Primitive solution vector
   * @param[in] A   Cross-sectional area
   */
  virtual std::vector<ADReal> computeFluxFromPrimitive(const std::vector<ADReal> & W,
                                               const ADReal & A) const = 0;

  /**
   * Gets a vector of pairs of primitive solution vectors to use for symmetry test
   */
  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionsSymmetryTest() const = 0;

  /**
   * Gets a vector of primitive solution vectors to use for consistency test
   */
  virtual std::vector<std::vector<ADReal>> getPrimitiveSolutionsConsistencyTest() const = 0;

  /**
   * Runs the consistency test(s)
   */
  void testConsistency();

  /**
   * Runs the symmetry test(s)
   */
  void testSymmetry();
};
