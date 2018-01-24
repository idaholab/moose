/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAINCENTERSPOSTPROCESSOR_H
#define GRAINCENTERSPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class GrainCentersPostprocessor;
class ComputeGrainCenterUserObject;

template <>
InputParameters validParams<GrainCentersPostprocessor>();

/**
 *  GrainCentersPostprocessor is a type of VectorPostprocessor that outputs center and volume of
 * grains
 *  calculated in GrainCenterUserObject.
 */
class GrainCentersPostprocessor : public GeneralVectorPostprocessor
{
public:
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

#endif // GRAINCENTERSPOSTPROCESSOR_H
