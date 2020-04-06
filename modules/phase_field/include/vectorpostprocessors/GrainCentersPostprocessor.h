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

// Forward Declarations
class ComputeGrainCenterUserObject;

/**
 *  GrainCentersPostprocessor is a type of VectorPostprocessor that outputs center and volume of
 * grains
 *  calculated in GrainCenterUserObject.
 */
class GrainCentersPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  GrainCentersPostprocessor(const InputParameters & parameters);

  virtual ~GrainCentersPostprocessor() {}
  virtual void initialize(){};
  virtual void execute();

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _grain_volume_center_vector;

  /// Userobject that calculates volumes and centers of grains
  const ComputeGrainCenterUserObject & _grain_data;
  /// Extracting grain volumes from Userobject
  const std::vector<Real> & _grain_volumes;
  /// Extracting grain centers from Userobject
  const std::vector<Point> & _grain_centers;

  unsigned int _total_grains;
};
