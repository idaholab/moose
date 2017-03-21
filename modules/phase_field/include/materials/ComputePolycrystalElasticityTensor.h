/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COMPUTEPOLYCRYSTALELASTICITYTENSOR_H
#define COMPUTEPOLYCRYSTALELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"
#include "GrainDataTracker.h"

// Forward Declarations
class ComputePolycrystalElasticityTensor;
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

  Real _length_scale;
  Real _pressure_scale;

  /// Grain tracker object
  const GrainDataTracker<RankFourTensor> & _grain_tracker;

  /// Number of order parameters
  const unsigned int _op_num;

  /// Order parameters
  std::vector<const VariableValue *> _vals;

  /// vector of elasticity tensor material properties
  std::vector<MaterialProperty<RankFourTensor> *> _D_elastic_tensor;

  /// Conversion factor from J to eV
  const Real _JtoeV;
};

#endif // COMPUTEPOLYCRYSTALELASTICITYTENSOR_H
