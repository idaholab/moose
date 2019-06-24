//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElasticityTensorBase.h"
#include "GrainDataTracker.h"
#include "RankFourTensor.h"

class ComputeBlockRotatedElasticityTensor;
class EulerAngleProvider;

template <>
InputParameters validParams<ComputeBlockRotatedElasticityTensor>();

/**
 * ComputeBlockRotatedElasticityTensor defines an elasticity tensor material object with a given
 * base name.
 */
class ComputeBlockRotatedElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputeBlockRotatedElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// unrotated elasticity tensor
  RankFourTensor _C_ijkl;

  /// object providing the Euler angles
  const EulerAngleProvider & _euler;
  
  // offsets the grain_id
  int _offset;
};
