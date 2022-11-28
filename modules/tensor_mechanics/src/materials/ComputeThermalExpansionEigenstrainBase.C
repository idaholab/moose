//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainBase.h"
#include "RankTwoTensor.h"

template <bool is_ad>
InputParameters
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::validParams()
{
  InputParameters params = ComputeEigenstrainBaseTempl<is_ad>::validParams();
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar("stress_free_temperature",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain "
                               "calculation");
  params.addParam<bool>("use_old_temperature",
                        false,
                        "Flag to optionally use the temperature value from the previous timestep.");
  params.addParam<MaterialPropertyName>("mean_thermal_expansion_coefficient_name",
                                        "Name of the mean coefficient of thermal expansion.");
  return params;
}

template <bool is_ad>
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::ComputeThermalExpansionEigenstrainBaseTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBaseTempl<is_ad>>(parameters),
    _temperature(_temperature_buffer),
    _use_old_temperature(this->template getParam<bool>("use_old_temperature")),
    _temperature_old(this->_fe_problem.isTransient() ? this->coupledValueOld("temperature")
                                                     : this->_zero),
    _deigenstrain_dT((is_ad || this->isCoupledConstant("temperature"))
                         ? nullptr
                         : &this->template declarePropertyDerivative<RankTwoTensor>(
                               _eigenstrain_name, this->coupledName("temperature"))),
    _stress_free_temperature(this->coupledValue("stress_free_temperature")),
    _temperature_prop(this->template coupledGenericValue<is_ad>("temperature")),
    _mean_thermal_expansion_coefficient(
        this->isParamValid("mean_thermal_expansion_coefficient_name")
            ? &this->template declareProperty<Real>(this->template getParam<MaterialPropertyName>(
                  "mean_thermal_expansion_coefficient_name"))
            : nullptr)
{
  if (_use_old_temperature && !this->_fe_problem.isTransient())
    this->paramError(
        "use_old_temperature",
        "The old state of the temperature variable is only available in a transient simulation.");
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::subdomainSetup()
{
  // call parent class subdomain setup, which ultimately calls Material::subdomainSetup()
  ComputeEigenstrainBaseTempl<is_ad>::subdomainSetup();

  // make sure we have enouch space to hold the augmented temperature values
  const auto nqp = this->_fe_problem.getMaxQps();
  _temperature_buffer.resize(nqp);
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::computeProperties()
{
  // we need to convert the temperature variable to a ChainedReal in the is_ad == false case
  for (_qp = 0; _qp < this->_qrule->n_points(); ++_qp)
    if constexpr (is_ad)
      _temperature_buffer[_qp] =
          _use_old_temperature ? _temperature_old[_qp] : _temperature_prop[_qp];
    else
    {
      if (_use_old_temperature)
        _temperature_buffer[_qp] = {_temperature_old[_qp], 0};
      else
        _temperature_buffer[_qp] = {_temperature_prop[_qp], 1};
    }

  ComputeEigenstrainBaseTempl<is_ad>::computeProperties();
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::computeQpEigenstrain()
{
  _eigenstrain[_qp].zero();
  const auto thermal_strain = computeThermalStrain();

  if constexpr (is_ad)
  {
    _eigenstrain[_qp].addIa(thermal_strain);
    if (_mean_thermal_expansion_coefficient)
    {
      if (_temperature[_qp] == _stress_free_temperature[_qp])
        (*_mean_thermal_expansion_coefficient)[_qp] = 0.0;
      else
        (*_mean_thermal_expansion_coefficient)[_qp] = MetaPhysicL::raw_value(
            thermal_strain / (_temperature[_qp] - _stress_free_temperature[_qp]));
    }
  }
  else
  {
    _eigenstrain[_qp].addIa(thermal_strain.value());
    if (_mean_thermal_expansion_coefficient)
    {
      if (_temperature[_qp].value() == _stress_free_temperature[_qp])
        (*_mean_thermal_expansion_coefficient)[_qp] = 0.0;
      else
        (*_mean_thermal_expansion_coefficient)[_qp] =
            thermal_strain.value() / (_temperature[_qp].value() - _stress_free_temperature[_qp]);
    }
    if (_deigenstrain_dT)
    {
      (*_deigenstrain_dT)[_qp].zero();
      if (!_use_old_temperature)
        (*_deigenstrain_dT)[_qp].addIa(thermal_strain.derivatives());
    }
  }
}

template class ComputeThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeThermalExpansionEigenstrainBaseTempl<true>;
