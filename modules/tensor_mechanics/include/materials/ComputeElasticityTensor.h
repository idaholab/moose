//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEELASTICITYTENSOR_H
#define COMPUTEELASTICITYTENSOR_H

#include "ComputeRotatedElasticityTensorBase.h"

class ComputeElasticityTensor;

template <>
InputParameters validParams<ComputeElasticityTensor>();

/**
 * ComputeElasticityTensor defines an elasticity tensor material object with a given base name.
 */
class ComputeElasticityTensor : public ComputeRotatedElasticityTensorBase
{
public:
  ComputeElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor() override;

  /// Individual material information
  RankFourTensor _Cijkl;
};

#endif // COMPUTEELASTICITYTENSOR_H
