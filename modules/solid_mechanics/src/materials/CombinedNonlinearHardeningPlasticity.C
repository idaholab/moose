#include "CombinedNonlinearHardeningPlasticity.h"
#include "Function.h"
#include "ElasticityTensorTools.h"
#include "RadialReturnBackstressStressUpdateBase.h"
#include "ElasticityTensorTools.h"

registerMooseObject("SolidMechanicsApp", ADCombinedNonlinearHardeningPlasticity);
registerMooseObject("SolidMechanicsApp", CombinedNonlinearHardeningPlasticity);

template <bool is_ad>
InputParameters
CombinedNonlinearHardeningPlasticityTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnBackstressStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("Combined isotropic and kinematic plasticity model with nonlinear "
                             "hardening rules, including a Voce model for isotropic hardening and "
                             "an Armstrong-Fredrick model for kinematic hardening.");
  params.addParam<Real>("q", 0.0, "Saturation value for isotropic hardening (Q in Voce model)");
  params.addParam<Real>("b", 0.0, "Rate constant for isotropic hardening (b in Voce model)");
  params.addParam<FunctionName>("yield_stress_function",
                                "Yield stress as a function of temperature");
  params.addParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addParam<FunctionName>("isotropic_hardening_function",
                                "True stress as a function of plastic strain");
  params.addParam<Real>("isotropic_hardening_constant", "Isotropic hardening slope");
  params.addCoupledVar("temperature", 0.0, "Coupled Temperature");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_plastic_strain";
  params.addParam<Real>("kinematic_hardening_modulus", 0.0, "Kinematic hardening modulus");
  params.addParam<Real>(
      "gamma", 0.0, "The nonlinear hardening parameter (gamma) for back stress evolution");

  return params;
}

template <bool is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::CombinedNonlinearHardeningPlasticityTempl(
    const InputParameters & parameters)
  : RadialReturnBackstressStressUpdateBaseTempl<is_ad>(parameters),
    _yield_stress_function(this->isParamValid("yield_stress_function")
                               ? &this->getFunction("yield_stress_function")
                               : nullptr),
    _yield_stress(this->isParamValid("yield_stress") ? this->template getParam<Real>("yield_stress")
                                                     : 0),
    _isotropic_hardening_constant(
        this->isParamValid("isotropic_hardening_constant")
            ? this->template getParam<Real>("isotropic_hardening_constant")
            : 0),
    _isotropic_hardening_function(this->isParamValid("isotropic_hardening_function")
                                      ? &this->getFunction("isotropic_hardening_function")
                                      : nullptr),
    _yield_condition(-1.0), // set to a non-physical value to catch uninitalized yield condition
    _isotropic_hardening_slope(0.0),
    _plastic_strain(
        this->template declareGenericProperty<RankTwoTensor, is_ad>(_base_name + "plastic_strain")),
    _plastic_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(_base_name + "plastic_strain")),
    _kinematic_hardening_modulus(this->template getParam<Real>("kinematic_hardening_modulus")),
    _gamma(this->template getParam<Real>("gamma")),
    _q(this->template getParam<Real>("q")),
    _b(this->template getParam<Real>("b")),
    _isotropic_hardening_variable(this->template declareGenericProperty<Real, is_ad>(
        _base_name + "isotropic_hardening_variable")),
    _isotropic_hardening_variable_old(
        this->template getMaterialPropertyOld<Real>(_base_name + "isotropic_hardening_variable")),
    _kinematic_hardening_variable(
        this->template declareGenericProperty<Real, is_ad>("kinematic_hardening_variable")),
    _kinematic_hardening_variable_old(
        this->template getMaterialPropertyOld<Real>("kinematic_hardening_variable")),
    _temperature(this->template coupledGenericValue<is_ad>("temperature"))
{
  if (parameters.isParamSetByUser("yield_stress") && _yield_stress <= 0.0)
    mooseError("Yield stress must be greater than zero");
  // Both of these parameters are given default values by derived classes, which makes them valid
  if (_yield_stress_function == nullptr && !this->isParamValid("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");
  if (!parameters.isParamValid("isotropic_hardening_constant") &&
      !this->isParamValid("isotropic_hardening_function"))
    mooseError(
        "Either isotropic_hardening_constant or isotropic_hardening_function must be defined");
  if (parameters.isParamSetByUser("isotropic_hardening_constant") &&
      this->isParamValid("isotropic_hardening_function"))
    mooseError(
        "Only the isotropic_hardening_constant or only the isotropic_hardening_function can be "
        "defined but not both");
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::initQpStatefulProperties()
{
  _isotropic_hardening_variable[_qp] = 0.0;
  _kinematic_hardening_variable[_qp] = 0.0;
  _plastic_strain[_qp].zero();
  RadialReturnBackstressStressUpdateBaseTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::propagateQpStatefulProperties()
{
  _isotropic_hardening_variable[_qp] = _isotropic_hardening_variable_old[_qp];
  _kinematic_hardening_variable[_qp] = _kinematic_hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  RadialReturnBackstressStressUpdateBaseTempl<is_ad>::propagateQpStatefulPropertiesRadialReturn();
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeStressInitialize(
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  RadialReturnBackstressStressUpdateBaseTempl<is_ad>::computeStressInitialize(
      effective_trial_stress, elasticity_tensor);

  computeYieldStress(elasticity_tensor);
  _yield_condition =
      effective_trial_stress - _isotropic_hardening_variable_old[_qp] - _yield_stress;
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
}

template <bool is_ad>
GenericReal<is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeResidual(
    const GenericReal<is_ad> & effective_trial_stress, const GenericReal<is_ad> & scalar)
{
  mooseAssert(_yield_condition != -1.0,
              "the yield stress was not updated by computeStressInitialize");
  if (_yield_condition > 0.0)
  {
    _isotropic_hardening_slope = computeIsotropicHardeningDerivative(scalar);
    _isotropic_hardening_variable[_qp] = computeIsotropicHardeningValue(scalar);
    _kinematic_hardening_variable[_qp] = computeKinematicHardeningValue(scalar);
    GenericReal<is_ad> residual = (effective_trial_stress - _kinematic_hardening_variable[_qp] -
                                   _isotropic_hardening_variable[_qp] - _yield_stress) /
                                      _three_shear_modulus -
                                  scalar;
    return residual;
  }
  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeDerivative(
    const GenericReal<is_ad> & /*effective_trial_stress*/, const GenericReal<is_ad> & /*scalar*/)
{
  if (_yield_condition > 0.0)
    return -1.0 - _isotropic_hardening_slope / _three_shear_modulus;
  return 1.0;
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::iterationFinalize(
    const GenericReal<is_ad> & scalar)
{
  if (_yield_condition > 0.0)
  {
    _isotropic_hardening_variable[_qp] = computeIsotropicHardeningValue(scalar);
    _kinematic_hardening_variable[_qp] = computeKinematicHardeningValue(scalar);
  }
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _plastic_strain[_qp] += plastic_strain_increment;
  this->_backstress[_qp] =
      this->_backstress_old[_qp] +
      (2.0 / 3.0) * _kinematic_hardening_modulus * plastic_strain_increment -
      _gamma * this->_backstress[_qp] * this->_effective_inelastic_strain_increment;
}

template <bool is_ad>
GenericReal<is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeIsotropicHardeningValue(
    const GenericReal<is_ad> & scalar)
{
  const Real _q = this->template getParam<Real>("q");
  const Real _b = this->template getParam<Real>("b");
  if (_isotropic_hardening_function)
  {
    const Real strain_old = this->_effective_inelastic_strain_old[_qp];
    return _isotropic_hardening_function->value(strain_old + scalar) - _yield_stress;
  }

  _isotropic_hardening_variable[_qp] = _q * (1.0 - std::exp(-_b * scalar));

  return (_isotropic_hardening_variable_old[_qp] + _isotropic_hardening_slope * scalar +
          _b * (_q - _isotropic_hardening_variable_old[_qp]) *
              this->_effective_inelastic_strain_increment);
}

template <bool is_ad>
GenericReal<is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeIsotropicHardeningDerivative(
    const GenericReal<is_ad> & /*scalar*/)
{
  if (_isotropic_hardening_function)
  {
    const Real strain_old = this->_effective_inelastic_strain_old[_qp];
    return _isotropic_hardening_function->timeDerivative(strain_old);
  }
  return _isotropic_hardening_constant;
}

template <bool is_ad>
GenericReal<is_ad>
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeKinematicHardeningValue(
    const GenericReal<is_ad> & scalar)
{
  _kinematic_hardening_variable[_qp] = _kinematic_hardening_modulus * scalar;
  return _kinematic_hardening_variable[_qp];
}

template <bool is_ad>
void
CombinedNonlinearHardeningPlasticityTempl<is_ad>::computeYieldStress(
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  if (_yield_stress_function)
  {
    static const Moose::GenericType<Point, is_ad> p;
    _yield_stress = _yield_stress_function->value(_temperature[_qp], p);
    if (_yield_stress <= 0.0)
      mooseError("In ",
                 this->_name,
                 ": The calculated yield stress (",
                 _yield_stress,
                 ") is less than zero");
  }
}

template class CombinedNonlinearHardeningPlasticityTempl<false>;
template class CombinedNonlinearHardeningPlasticityTempl<true>;
