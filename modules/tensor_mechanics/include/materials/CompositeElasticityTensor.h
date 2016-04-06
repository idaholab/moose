/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPOSITEELASTICITYTENSOR_H
#define COMPOSITEELASTICITYTENSOR_H

#include "CompositeTensorBase.h"
#include "RankFourTensor.h"

/**
 * CompositeElasticityTensor provides a simple RankFourTensor type
 * MaterialProperty that can be used as an Elasticity tensor in a mechanics simulation.
 * This tensor is computes as a weighted sum of base elasticity tensors where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeElasticityTensor : public CompositeTensorBase<RankFourTensor>
{
public:
  CompositeElasticityTensor(const InputParameters & parameters);

protected:
  std::string _base_name;
};

template<>
InputParameters validParams<CompositeElasticityTensor>();

#endif //COMPOSITEELASTICITYTENSOR_H
