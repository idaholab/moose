//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRadialReturnStressUpdate.h"

/**
 * This class uses the Discrete material in a radial return isotropic plasticity
 * model.  This class is one of the basic radial return constitutive models;
 * more complex constitutive models combine creep and plasticity.
 *
 * This class inherits from RadialReturnStressUpdate and must be used
 * in conjunction with ComputeReturnMappingStress.  This class calculates
 * an effective trial stress, an effective scalar plastic strain
 * increment, and the derivative of the scalar effective plastic strain increment;
 * these values are passed to the RadialReturnStressUpdate to compute
 * the radial return stress increment.  This isotropic plasticity class also
 * computes the plastic strain as a stateful material property.
 *
 * This class is based on the implicit integration algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University
 * Press, pg. 146 - 149.
 */
class ADIsotropicPlasticityStressUpdate : public ADRadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  ADIsotropicPlasticityStressUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void computeStressInitialize(const ADReal & effective_trial_stress,
                                       const ADRankFourTensor & elasticity_tensor) override;
  virtual ADReal computeResidual(const ADReal & effective_trial_stress,
                                 const ADReal & scalar) override;
  virtual ADReal computeDerivative(const ADReal & effective_trial_stress,
                                   const ADReal & scalar) override;
  virtual void iterationFinalize(ADReal scalar) override;
  virtual void computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment) override;

  virtual void computeYieldStress(const ADRankFourTensor & elasticity_tensor);
  virtual ADReal computeHardeningValue(const ADReal & scalar);
  virtual ADReal computeHardeningDerivative(const ADReal & scalar);

  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  const Function * const _yield_stress_function;
  ADReal _yield_stress;
  const Real _hardening_constant;
  const Function * const _hardening_function;

  ADReal _yield_condition;
  ADReal _hardening_slope;

  /// plastic strain in this model
  ADMaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  ADMaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
  const ADVariableValue & _temperature;
};
