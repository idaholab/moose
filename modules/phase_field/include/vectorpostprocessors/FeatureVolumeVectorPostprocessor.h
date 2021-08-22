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
#include "MooseVariableDependencyInterface.h"
#include "BoundaryRestrictable.h"

#include <array>

// Forward Declarations
class FeatureFloodCount;

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
                                         public MooseVariableDependencyInterface,
                                         public BoundaryRestrictable
{
public:
  static InputParameters validParams();

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
  const bool _output_centroids;

  /// A reference to the feature flood count object
  const FeatureFloodCount & _feature_counter;

  VectorPostprocessorValue & _var_num;
  VectorPostprocessorValue & _feature_volumes;
  VectorPostprocessorValue & _intersects_bounds;
  VectorPostprocessorValue & _intersects_specified_bounds;
  VectorPostprocessorValue & _percolated;

  /// Indicates whether the calculation should be run on volumes or area of a boundary
  bool _is_boundary_restricted;

private:
  /// Add volume contributions to one or entries in the feature volume vector
  void accumulateVolumes(const Elem * elem,
                         const std::vector<unsigned int> & var_to_features,
                         std::size_t num_features);

  /// When boundary is supplied as input, compute coverage of that boundary by each feature
  void accumulateBoundaryFaces(const Elem * elem,
                               const std::vector<unsigned int> & var_to_features,
                               std::size_t num_features,
                               unsigned int side);

  /// Calculate the integral value of the passed in variable (index)
  Real computeIntegral(std::size_t var_index) const;

  /// Calculate the integral on the face if boundary is supplied as input
  Real computeFaceIntegral(std::size_t var_index) const;

  const std::vector<MooseVariableFEBase *> & _vars;
  std::vector<const VariableValue *> _coupled_sln;

  MooseMesh & _mesh;
  Assembly & _assembly;
  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const QBase * const & _qrule_face;
  const MooseArray<Real> & _JxW_face;

  std::array<VectorPostprocessorValue *, 3> _centroid;
};
