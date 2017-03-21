/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEELASTICITYTENSOR_H
#define COMPUTEELASTICITYTENSOR_H

#include "ComputeRotatedElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object with a given base name.
 */
class ComputeElasticityTensor : public ComputeRotatedElasticityTensorBase
{
public:
  ComputeElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Individual material information
  RankFourTensor _Cijkl;
};

#endif // COMPUTEELASTICITYTENSOR_H
