//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnStressUpdate.h"

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

template <bool is_ad>
class IsotropicPlasticityStressUpdateTempl : public RadialReturnStressUpdateTempl<is_ad>
{
public:
  static InputParameters validParams();

  IsotropicPlasticityStressUpdateTempl(const InputParameters & parameters);

  using Material::_qp;
  using RadialReturnStressUpdateTempl<is_ad>::_base_name;
  using RadialReturnStressUpdateTempl<is_ad>::_three_shear_modulus;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void
  computeStressInitialize(const GenericReal<is_ad> & effective_trial_stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor) override;
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                             const GenericReal<is_ad> & scalar) override;
  virtual GenericReal<is_ad> computeDerivative(const GenericReal<is_ad> & effective_trial_stress,
                                               const GenericReal<is_ad> & scalar) override;
  virtual void iterationFinalize(GenericReal<is_ad> scalar) override;
  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & plastic_strain_increment) override;

  virtual void computeYieldStress(const GenericRankFourTensor<is_ad> & elasticity_tensor);
  virtual GenericReal<is_ad> computeHardeningValue(const GenericReal<is_ad> & scalar);
  virtual GenericReal<is_ad> computeHardeningDerivative(const GenericReal<is_ad> & scalar);

  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  const Function * _yield_stress_function;
  GenericReal<is_ad> _yield_stress;
  const Real _hardening_constant;
  const Function * const _hardening_function;

  GenericReal<is_ad> _yield_condition;
  GenericReal<is_ad> _hardening_slope;

  /// plastic strain in this model
  GenericMaterialProperty<RankTwoTensor, is_ad> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  GenericMaterialProperty<Real, is_ad> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
  const GenericVariableValue<is_ad> & _temperature;
};

typedef IsotropicPlasticityStressUpdateTempl<false> IsotropicPlasticityStressUpdate;
typedef IsotropicPlasticityStressUpdateTempl<true> ADIsotropicPlasticityStressUpdate;
