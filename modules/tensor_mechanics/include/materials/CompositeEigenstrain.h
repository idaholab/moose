/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPOSITEEIGENSTRAIN_H
#define COMPOSITEEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"
#include "CompositeTensorBase.h"
#include "RankTwoTensor.h"

/**
 * CompositeEigenstrain provides a simple RankTwoTensor type
 * MaterialProperty that can be used as an Eigenstrain tensor in a mechanics simulation.
 * This tensor is computes as a weighted sum of base Eigenstrain tensors where each weight
 * can be a scalar material property that may depend on simulation variables.
 * The generic logic that computes a weighted sum of tensors is located in the
 * templated base class CompositeTensorBase.
 */
class CompositeEigenstrain : public CompositeTensorBase<RankTwoTensor, ComputeEigenstrainBase>
{
public:
  CompositeEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const std::string _M_name;
};

template <>
InputParameters validParams<CompositeEigenstrain>();

#endif // COMPOSITEEIGENSTRAIN_H
