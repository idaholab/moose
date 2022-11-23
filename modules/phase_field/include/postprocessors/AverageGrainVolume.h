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
#include "MooseVariableDependencyInterface.h"

// Forward Declarations
class FeatureFloodCount;
class MooseMesh;

/**
 * Compute the average grain area in a polycrystal
 */
class AverageGrainVolume : public GeneralPostprocessor,
                           public Coupleable,
                           public MooseVariableDependencyInterface
{
public:
  static InputParameters validParams();

  AverageGrainVolume(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;

protected:
  void accumulateVolumes(const std::vector<unsigned int> & var_to_features,
                         std::size_t libmesh_dbg_var(num_features));
  Real computeIntegral(std::size_t var_index) const;

private:
  /// A reference to the mesh
  MooseMesh & _mesh;
  Assembly & _assembly;
  std::vector<unsigned int> _static_var_to_feature;
  std::vector<const VariableValue *> _vals;
  std::vector<Real> _feature_volumes;
  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const FeatureFloodCount * const _feature_counter;
};
