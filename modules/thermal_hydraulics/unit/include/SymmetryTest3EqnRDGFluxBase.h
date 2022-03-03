//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Test3EqnRDGObjectBase.h"

/**
 * Base class for testing symmetry of a numerical flux for the 3-equation model.
 *
 * This class is used to test the following symmetry property of a numerical flux:
 * \f[
 *   F(U_L, U_R, n_x) = F(U_R, U_L, -n_x) \,.
 * \f]
 */
class SymmetryTest3EqnRDGFluxBase : public Test3EqnRDGObjectBase
{
public:
  SymmetryTest3EqnRDGFluxBase() : Test3EqnRDGObjectBase() {}

protected:
  /**
   * Gets a vector of pairs of primitive solution vectors to use for tests
   */
  virtual std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
  getPrimitiveSolutionPairs() const = 0;

  virtual void test() override;
};
