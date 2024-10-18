#pragma once

#include "MaterialProperty.h"
#include "RadialReturnBackstressStressUpdateBase.h"

template <bool is_ad>
class CombinedNonlinearHardeningPlasticityTempl
  : public RadialReturnBackstressStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  CombinedNonlinearHardeningPlasticityTempl(const InputParameters & parameters);

  using Material::_qp;
  using RadialReturnBackstressStressUpdateBaseTempl<is_ad>::_base_name;
  using RadialReturnBackstressStressUpdateBaseTempl<is_ad>::_three_shear_modulus;

  virtual void
  computeStressInitialize(const GenericReal<is_ad> & effective_trial_stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor) override;
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                             const GenericReal<is_ad> & scalar) override;
  virtual GenericReal<is_ad> computeDerivative(const GenericReal<is_ad> & effective_trial_stress,
                                               const GenericReal<is_ad> & scalar) override;

  virtual void computeYieldStress(const GenericRankFourTensor<is_ad> & elasticity_tensor);

  GenericReal<is_ad> yieldCondition() const { return _yield_condition; }

  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & plastic_strain_increment) override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void iterationFinalize(const GenericReal<is_ad> & scalar) override;

  virtual GenericReal<is_ad> computeIsotropicHardeningValue(const GenericReal<is_ad> & scalar);
  virtual GenericReal<is_ad> computeIsotropicHardeningDerivative(const GenericReal<is_ad> & scalar);
  virtual GenericReal<is_ad> computeKinematicHardeningValue(const GenericReal<is_ad> & scalar);

  const Function * _yield_stress_function;
  GenericReal<is_ad> _yield_stress;
  const Real _isotropic_hardening_constant;
  const Function * const _isotropic_hardening_function;

  GenericReal<is_ad> _yield_condition;
  GenericReal<is_ad> _isotropic_hardening_slope;
  GenericReal<is_ad> _kinematic_hardening_slope;
  GenericRankTwoTensor<is_ad> stress_new;
  GenericRankTwoTensor<is_ad> elastic_strain_old;
  GenericRankTwoTensor<is_ad> strain_increment;

  /// plastic strain
  GenericMaterialProperty<RankTwoTensor, is_ad> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  ///@{  Coefficients for combined isotropic and kinematic hardening
  const Real _kinematic_hardening_modulus;
  const Real _gamma;
  const Real _q;
  const Real _b;
  ///@}

  GenericMaterialProperty<Real, is_ad> & _isotropic_hardening_variable;
  const MaterialProperty<Real> & _isotropic_hardening_variable_old;
  GenericMaterialProperty<Real, is_ad> & _kinematic_hardening_variable;
  const MaterialProperty<Real> & _kinematic_hardening_variable_old;
  const GenericVariableValue<is_ad> & _temperature;
};

typedef CombinedNonlinearHardeningPlasticityTempl<false> CombinedNonlinearHardeningPlasticity;
typedef CombinedNonlinearHardeningPlasticityTempl<true> ADCombinedNonlinearHardeningPlasticity;
