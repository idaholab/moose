/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FEATUREVOLUMEVECTORPOSTPROCESSOR_H
#define FEATUREVOLUMEVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"
#include "MooseVariableDependencyInterface.h"

// Forward Declarations
class FeatureVolumeVectorPostprocessor;
class FeatureFloodCount;

template <>
InputParameters validParams<FeatureVolumeVectorPostprocessor>();

/**
 * This VectorPostprocessor is intended to be used to calculate
 * accurate volumes from the FeatureFloodCount and/or GrainTracker
 * objects. It is a GeneralVectorPostProcessor instead of the
 * more natural elemental kind so that dependency resolution
 * will work properly when an AuxVariable is not depending
 * on the FeatureFloodCount object. It obtains the coupled
 * variables from the FeatureFloodCount object so that there's
 * one less thing for the user of this class to worry about.
 */
class FeatureVolumeVectorPostprocessor : public GeneralVectorPostprocessor,
                                         public MooseVariableDependencyInterface
{
public:
  FeatureVolumeVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  /**
   * Returns the volume for the given grain number.
   */
  Real getFeatureVolume(unsigned int feature_id) const;

protected:
  /// A Boolean indicating how the volume is calculated
  const bool _single_feature_per_elem;

  /// A reference to the feature flood count object
  const FeatureFloodCount & _feature_counter;

  VectorPostprocessorValue & _var_num;
  VectorPostprocessorValue & _feature_volumes;
  VectorPostprocessorValue & _intersects_bounds;

private:
  /// Add volume contributions to one or entries in the feature volume vector
  void accumulateVolumes(const Elem * elem,
                         const std::vector<unsigned int> & var_to_features,
                         std::size_t num_features);

  /// Calculate the integral value of the passed in variable (index)
  Real computeIntegral(std::size_t var_index) const;

  const std::vector<MooseVariable *> & _vars;
  std::vector<const VariableValue *> _coupled_sln;

  MooseMesh & _mesh;
  Assembly & _assembly;
  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
};

#endif
