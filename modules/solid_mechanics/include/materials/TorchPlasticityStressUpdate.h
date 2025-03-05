//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "RadialReturnStressUpdate.h"

class TorchScriptUserObject;

class TorchPlasticityStressUpdate : public RadialReturnStressUpdateTempl<false>
{
public:
  static InputParameters validParams();

  TorchPlasticityStressUpdate(const InputParameters & parameters);

  using Material::_qp;
  using RadialReturnStressUpdateTempl<false>::_base_name;
  using RadialReturnStressUpdateTempl<false>::_three_shear_modulus;

  virtual void
  computeStressInitialize(const Real & effective_trial_stress,
                          const RankFourTensor & elasticity_tensor) override;
  virtual Real computeResidual(const Real & effective_trial_stress,
                                             const Real & scalar) override;
  virtual Real computeDerivative(const Real & effective_trial_stress,
                                               const Real & scalar) override;

  virtual void computeYieldStress(const RankFourTensor & elasticity_tensor);

  Real yieldCondition() const { return _yield_condition; }

  virtual void
  computeStressFinalize(const RankTwoTensor & plastic_strain_increment) override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void iterationFinalize(const Real & scalar) override;

  virtual Real computeHardeningValue(const Real & scalar);
  virtual Real computeHardeningDerivative(const Real & scalar);

  void computeHardeningSlope();

  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  const Function * _yield_stress_function;
  Real _yield_stress;

  /// The user object that holds the neural network
  const TorchScriptUserObject & _dislocation_density;

  Real _yield_condition;
  Real _hardening_slope;

  /// plastic strain in this model
  GenericMaterialProperty<RankTwoTensor, false> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  GenericMaterialProperty<Real, false> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
  const GenericVariableValue<false> & _temperature;
};

#endif