/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FEATUREBOUNDINGBOXVECTORPOSTPROCESSOR_H
#define FEATUREBOUNDINGBOXVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"
#include "MooseVariableDependencyInterface.h"

//Forward Declarations
class FeatureBoundingBoxVectorPostprocessor;
class FeatureFloodCount;

template<>
InputParameters validParams<FeatureBoundingBoxVectorPostprocessor>();

/**
 * This VectorPostprocessor is intended to output all of the
 * bounding boxes for features identificed by the feature
 * flood count object. It is separate from the Volume VPP
 * because each feature may have several bboxes in periodic
 * boundary conditions are used.
 */
class FeatureBoundingBoxVectorPostprocessor :
  public GeneralVectorPostprocessor,
  public MooseVariableDependencyInterface
{
public:
  FeatureBoundingBoxVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// A reference to the feature flood count object
  const FeatureFloodCount & _feature_counter;

  VectorPostprocessorValue & _feature_id;
  VectorPostprocessorValue & _min_x;
  VectorPostprocessorValue & _min_y;
  VectorPostprocessorValue & _min_z;
  VectorPostprocessorValue & _max_x;
  VectorPostprocessorValue & _max_y;
  VectorPostprocessorValue & _max_z;

private:
  /// Add volume contributions to one or entries in the feature volume vector
  void accumulateVolumes(const Elem * elem, const std::vector<unsigned int> & var_to_features, std::size_t num_features);

  /// Calculate the integral value of the passed in variable (index)
  Real computeIntegral(std::size_t var_index) const;

  const std::vector<MooseVariable *> & _vars;
};

#endif
