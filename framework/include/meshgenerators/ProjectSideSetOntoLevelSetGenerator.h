//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

#include "FunctionParserUtils.h"

/**
 * Projects a sideset onto a level set function using a fixed vector
 */
class ProjectSideSetOntoLevelSetGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ProjectSideSetOntoLevelSetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the input mesh
  const MeshGeneratorName _input_name;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
  /// Projection direction
  Point _proj_dir;
  /// The analytic level set function in the form of a string that can be parsed by FParser
  const std::string _level_set;
  /// function parser object describing the level set
  SymFunctionPtr _func_level_set;

  /**
   * Calculate the intersection point of a level set and a line segment defined by two
   * points separated by the level set.
   * @param point1 The first point of the line segment
   * @param point2 The second point of the line segment
   * @return the intersection point of the level set and the line segment
   */
  Point pointPairLevelSetInterception(const Point & point1, const Point & point2);

  /**
   * Evaluate the level set function at a given point.
   * @param point The point at which the level set function is to be evaluated
   * @return the value of the level set function at the given point
   */
  Real levelSetEvaluator(const Point & point);
};
