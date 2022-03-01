//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 *  SpatialUserObjectVectorPostprocessor is a type of VectorPostprocessor that outputs the
 *  values of a spatial user object as a vector in the order of the provided points.
 */
class SpatialUserObjectVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  SpatialUserObjectVectorPostprocessor(const InputParameters & parameters);

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize() override;

  /**
   * Populates the postprocessor vector of values with the userobject evaluations in space
   */
  virtual void execute() override;

  /**
   * Read the points at which to evaluate from a vector ('points'), a file ('points_file'),
   * or neither (which will read from the user object directly if it satisfies the
   * spatialPoints interface)
   */
  void fillPoints();

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _uo_vec;

  /// Userobject to evaluate spatially
  const UserObject & _uo;

  /// Points at which to evaluate the user object
  std::vector<Point> _points;
};
