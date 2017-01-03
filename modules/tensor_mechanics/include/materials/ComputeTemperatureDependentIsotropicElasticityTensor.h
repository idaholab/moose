/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
#define COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

class Function;

/**
 * ComputeTemperatureDependentIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials in which the elastic constants vary with temperature.
 */
class ComputeTemperatureDependentIsotropicElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputeTemperatureDependentIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpElasticityTensor();

  /// Temperature
  const VariableValue & _temperature;

  /// Function for the temperature dependence of the Young's modulus
  Function & _youngs_modulus_function;

  /// Function for the temperature dependence of the Poisson's ratio
  Function & _poissons_ratio_function;

  /// Store the old elasticity tensor to compute the stress correctly
  MaterialProperty<RankFourTensor> & _elasticity_tensor_old;
};

#endif //COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
