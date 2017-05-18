/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AVERAGEGRAINVOLUME_H
#define AVERAGEGRAINVOLUME_H

#include "GeneralPostprocessor.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"

// Forward Declarations
class FeatureFloodCount;
class AverageGrainVolume;
class MooseMesh;
template <>
InputParameters validParams<AverageGrainVolume>();

/**
 * Compute the average grain area in a polycrystal
 */
class AverageGrainVolume : public GeneralPostprocessor,
                           public Coupleable,
                           public MooseVariableDependencyInterface
{
public:
  AverageGrainVolume(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
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
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const FeatureFloodCount * _feature_counter;
};
#endif // AVERAGEGRAINVOLUME_H
