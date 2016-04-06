/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEPOLYCRYSTALELASTICITYTENSOR_H
#define COMPUTEPOLYCRYSTALELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

//Forward Declarations
class ComputePolycrystalElasticityTensor;
class GrainTracker;
class EulerAngleProvider;

/**
 * Compute an evolving elasticity tensor coupled to a grain growth phase field model.
 */
class ComputePolycrystalElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputePolycrystalElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Unrotated elasticity tensor
  RankFourTensor _C_unrotated;

  Real _length_scale;
  Real _pressure_scale;

  /// Object providing the Euler angles
  const EulerAngleProvider & _euler;

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// Number of grains
  unsigned int _grain_num;

  /// Number of order parameters
  unsigned int _nop;

  /// Number of extra euler angles that are stored for when grains are created
  unsigned int _stiffness_buffer;

  /// Order parameters
  std::vector<const VariableValue *> _vals;

  /// vector of elasticity tensor material properties
  std::vector< MaterialProperty<RankFourTensor> *> _D_elastic_tensor;

  /// vector of elasticity tensors for storing rotated elasticity tensors for each grain
  std::vector<RankFourTensor> _C_rotated;

  /// Conversion factor from J to eV
  const Real _JtoeV;
};

#endif //COMPUTEPOLYCRYSTALELASTICITYTENSOR_H
