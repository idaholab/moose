//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "Coupleable.h"

/**
 * Find a specific target value along a sampling line. The variable values along
 * the line should change monotonically. The target value is searched using a
 * bisection algorithm.
 * The Postprocessor reports the distance from the start_point along the line
 * between start_point and end_point.
 */
class FindValueOnLine : public GeneralPostprocessor, public Coupleable
{
public:
  static InputParameters validParams();

  FindValueOnLine(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual PostprocessorValue getValue() override;

protected:
  Real getValueAtPoint(const Point & p);

  ///@{ line to sample along
  const Point _start_point;
  const Point _end_point;
  const Real _length;
  ///@}

  /// value to find along the line
  const Real _target;

  /// boolean indicating whether to stop with an error if value is not found on the line
  const bool & _error_if_not_found;

  /// value to return if target value is not found on the line and _error_if_not_found is false
  const Real & _default_value;

  /// search depth
  const unsigned int _depth;

  /// tolerance for comparison to the target value
  const Real _tol;

  /// coupled variable
  MooseVariable & _coupled_var;

  /// detected interface location
  Real _position;

  /// The Mesh we're using
  MooseMesh & _mesh;

  /// So we don't have to create and destroy the dummy vector
  std::vector<Point> _point_vec;

  /// helper object to locate elements containing points
  std::unique_ptr<PointLocatorBase> _pl;
};
