//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionParserUtils.h"
#include "InputParameters.h"

/**
 * Helper class to define, parameterize and create a level set function used in meshing,
 * often to correct the position of nodes on a surface
 */
class LevelSetMeshingHelper : public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  LevelSetMeshingHelper(const InputParameters & parameters);

protected:
  /// Maximum number of iterations to correct the nodes based on the level set function
  const unsigned int _max_level_set_correction_iterations;

  /// function parser object describing the level set
  SymFunctionPtr _func_level_set;

  /**
   * Evaluate the level set function at a given point.
   * @param point The point at which the level set function is to be evaluated
   * @return the value of the level set function at the given point
   */
  Real levelSetEvaluator(const Point & point);

  /**
   * Correct the position of a node based on the level set function.
   * @param node The node to be corrected
   */
  void levelSetCorrection(Node & node);
};
