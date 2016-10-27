/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
#define COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H

#include "ComputeIsotropicElasticityTensor.h"

/**
 * ComputeVariableIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials in which the elastic constants vary with temperature.  This
 * class exists as a basic test functionality for more complex elasticity tensor
 * classes in various MOOSE Apps.
 */
class ComputeVariableIsotropicElasticityTensor : public ComputeIsotropicElasticityTensor
{
public:
  ComputeVariableIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpElasticityTensor();

  /// Temperature in Kelvin
  const VariableValue & _temperature;

  /// Store the old elasticity tensor to compute the stress correctly
  MaterialProperty<RankFourTensor> & _elasticity_tensor_old;
};

#endif //COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
