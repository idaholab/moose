/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECONCENTRATIONDEPENDENTELASTICITYTENSOR_H
#define COMPUTECONCENTRATIONDEPENDENTELASTICITYTENSOR_H

#include "ComputeRotatedElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object as a function of
 * concentration field.
 */
class ComputeConcentrationDependentElasticityTensor : public ComputeRotatedElasticityTensorBase
{
public:
  ComputeConcentrationDependentElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  /// Elasticity tensor for phase with zero concentration.
  RankFourTensor _Cijkl0;
  /// Elasticity tensor for phase with concentration 1.
  RankFourTensor _Cijkl1;
  /// Concentration variable.
  const VariableValue & _c;
  VariableName _c_name;

  /// Derivative of elasticity tensor with respect to concentration.
  MaterialProperty<RankFourTensor> & _delasticity_tensor_dc;
};

#endif // COMPUTECONCENTRATIONDEPENDENTELASTICITYTENSOR_H
