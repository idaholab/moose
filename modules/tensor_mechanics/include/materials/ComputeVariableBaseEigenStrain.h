/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVARIABLEBASEEIGENSTRAIN_H
#define COMPUTEVARIABLEBASEEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

class ComputeVariableBaseEigenStrain;

template <>
InputParameters validParams<ComputeVariableBaseEigenStrain>();

/**
 * ComputeVariableBaseEigenstrain computes an Eigenstrain based on a real tensor value material
 * property base (a),
 * a real material property prefactor (p) and a rank two tensor offset tensor (b)
 * p * a + b
 */
class ComputeVariableBaseEigenStrain : public ComputeEigenstrainBase
{
public:
  ComputeVariableBaseEigenStrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  const MaterialProperty<RealTensorValue> & _base_tensor;
  const MaterialProperty<Real> & _prefactor;
  RankTwoTensor _offset_tensor;
};

#endif
