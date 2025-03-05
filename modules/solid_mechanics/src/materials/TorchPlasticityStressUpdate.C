//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "TorchPlasticityStressUpdate.h"

#include "Function.h"
#include "ElasticityTensorTools.h"
#include "TorchScriptUserObject.h"
#include "LibtorchUtils.h"

registerMooseObject("SolidMechanicsApp", TorchPlasticityStressUpdate);

InputParameters
TorchPlasticityStressUpdate::validParams()
{
  InputParameters params = RadialReturnStressUpdateTempl<false>::validParams();
  params.addClassDescription("Example for a torch-based material model.");
  // Linear strain hardening parameters
  params.addParam<FunctionName>("yield_stress_function",
                                "Yield stress as a function of temperature");
  params.addParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addParam<UserObjectName>("dislocation_density", "DD UO.");
  params.addCoupledVar("temperature", 0.0, "Coupled Temperature");
  params.addDeprecatedParam<std::string>(
      "plastic_prepend",
      "",
      "String that is prepended to the plastic_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_plastic_strain";

  return params;
}

TorchPlasticityStressUpdate::TorchPlasticityStressUpdate(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<false>(parameters),
    _plastic_prepend(this->template getParam<std::string>("plastic_prepend")),
    _yield_stress_function(this->isParamValid("yield_stress_function")
                               ? &this->getFunction("yield_stress_function")
                               : nullptr),
    _yield_stress(this->isParamValid("yield_stress") ? this->template getParam<Real>("yield_stress")
                                                     : 0),
    _dislocation_density(getUserObject<TorchScriptUserObject>("dislocation_density")),
    _yield_condition(-1.0), // set to a non-physical value to catch uninitalized yield condition
    _plastic_strain(this->template declareGenericProperty<RankTwoTensor, false>(
        _base_name + _plastic_prepend + "plastic_strain")),
    _plastic_strain_old(this->template getMaterialPropertyOld<RankTwoTensor>(
        _base_name + _plastic_prepend + "plastic_strain")),
    _hardening_variable(
        this->template declareGenericProperty<Real, false>(_base_name + "hardening_variable")),
    _hardening_variable_old(
        this->template getMaterialPropertyOld<Real>(_base_name + "hardening_variable")),
    _temperature(this->template coupledGenericValue<false>("temperature"))
{
  if (parameters.isParamSetByUser("yield_stress") && _yield_stress <= 0.0)
    mooseError("Yield stress must be greater than zero");

  // Both of these parameters are given default values by derived classes, which makes them valid
  if (_yield_stress_function == nullptr && !this->isParamValid("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");
}

void
TorchPlasticityStressUpdate::initQpStatefulProperties()
{
  _hardening_variable[_qp] = 0.0;
  _plastic_strain[_qp].zero();
}

void
TorchPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  RadialReturnStressUpdateTempl<false>::propagateQpStatefulPropertiesRadialReturn();
}

void
TorchPlasticityStressUpdate::computeStressInitialize(
    const Real & effective_trial_stress,
    const RankFourTensor & elasticity_tensor)
{
  RadialReturnStressUpdateTempl<false>::computeStressInitialize(effective_trial_stress,
                                                                elasticity_tensor);

  computeYieldStress(elasticity_tensor);

  _yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
}

Real
TorchPlasticityStressUpdate::computeResidual(
    const Real & effective_trial_stress, const Real & scalar)
{
  mooseAssert(_yield_condition != -1.0,
              "the yield stress was not updated by computeStressInitialize");

  if (_yield_condition > 0.0)
  {
    
    _hardening_slope = computeHardeningDerivative(scalar);
    _hardening_variable[_qp] = computeHardeningValue(scalar);

    return (effective_trial_stress - _hardening_variable[_qp] - _yield_stress) /
               _three_shear_modulus -
           scalar;
  }

  return 0.0;
}

Real
TorchPlasticityStressUpdate::computeDerivative(
    const Real & /*effective_trial_stress*/, const Real & /*scalar*/)
{
  if (_yield_condition > 0.0)
    return -1.0 - _hardening_slope / _three_shear_modulus;

  return 1.0;
}

void
TorchPlasticityStressUpdate::iterationFinalize(const Real & scalar)
{
  if (_yield_condition > 0.0)
    _hardening_variable[_qp] = computeHardeningValue(scalar);
}

void
TorchPlasticityStressUpdate::computeStressFinalize(
    const RankTwoTensor & plastic_strain_increment)
{
  _plastic_strain[_qp] += plastic_strain_increment;
}

Real
TorchPlasticityStressUpdate::computeHardeningValue(
    const Real & scalar)
{
  std::vector<Real> input_vector(2, 0.0);
  input_vector[0] = this->_effective_inelastic_strain_old[_qp] + scalar;
  input_vector[1] = this->_temperature[_qp];

  torch::Tensor input_tensor;
  LibtorchUtils::vectorToTensor(input_vector, input_tensor);

  torch::Tensor dislocation_density = _dislocation_density.evaluate(input_tensor);

  // Random function, we can make this whatever we want in the future
  torch::Tensor hardening_value = dislocation_density + 50.0; 

  return hardening_value.item<Real>() - _yield_stress;
}

Real
TorchPlasticityStressUpdate::computeHardeningDerivative(
    const Real & /*scalar*/)
{
  std::vector<Real> input_vector(2, 0.0);
  input_vector[0] = this->_effective_inelastic_strain_old[_qp];
  input_vector[1] = this->_temperature[_qp];

  torch::Tensor input_tensor;
  LibtorchUtils::vectorToTensor(input_vector, input_tensor);

  input_tensor.requires_grad();

  torch::Tensor dislocation_density = _dislocation_density.evaluate(input_tensor);

  // Random function, we can make this whatever we want in the future
  torch::Tensor hardening_value = dislocation_density + 50.0;
  
  // We get the gradients of the hardening_value with respect to the 
  // strain
  hardening_value.backward();  

  return hardening_value.grad().index({1,0}).item<Real>();
}

void
TorchPlasticityStressUpdate::computeYieldStress(
    const RankFourTensor & /*elasticity_tensor*/)
{
  if (_yield_stress_function)
  {
    static const Point p;
    _yield_stress = _yield_stress_function->value(_temperature[_qp], p);

    if (_yield_stress <= 0.0)
      mooseError("In ",
                 this->_name,
                 ": The calculated yield stress (",
                 _yield_stress,
                 ") is less than zero");
  }
}

#endif