/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPOSITEMOBILITYTENSOR_H
#define COMPOSITEMOBILITYTENSOR_H

#include "Material.h"
#include "CompositeTensorBase.h"

/**
 * CompositeMobilityTensor provides a simple RealTensorValue type
 * MaterialProperty that can be used as a mobility in a phase field simulation.
 * This mobility is computes as a weighted sum of base mobilities where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeMobilityTensor : public CompositeTensorBase<RealTensorValue, Material>
{
public:
  CompositeMobilityTensor(const InputParameters & parameters);

protected:
  void computeQpProperties();

  const std::string _M_name;
  MaterialProperty<RealTensorValue> & _M;
};

template <>
InputParameters validParams<CompositeMobilityTensor>();

#endif // COMPOSITEMOBILITYTENSOR_H
