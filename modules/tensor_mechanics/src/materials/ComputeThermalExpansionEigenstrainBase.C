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
  return params;
}

template <bool is_ad>
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::ComputeThermalExpansionEigenstrainBaseTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBaseTempl<is_ad>>(parameters),
    _use_old_temperature(this->template getParam<bool>("use_old_temperature")),
    _temperature_old(_use_old_temperature ? this->coupledValueOld("temperature") : this->_zero),
    _temperature(this->template coupledGenericValue<is_ad>("temperature")),
    _deigenstrain_dT(std::invoke(
        [&]() -> decltype(auto)
        {
          if constexpr (is_ad)
            return nullptr;
          else
            return this->template declarePropertyDerivative<RankTwoTensor>(
                _eigenstrain_name, this->getVar("temperature", 0)->name());
        })),
    _stress_free_temperature(this->coupledValue("stress_free_temperature"))
{
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::computeQpEigenstrain()
{
  GenericReal<is_ad> thermal_strain = 0.0;

  if constexpr (is_ad)
    computeThermalStrain(thermal_strain);
  else
  {
    // instantaneous_cte is just the derivative of thermal_strain with respect to temperature
    Real instantaneous_cte = 0.0;
    computeThermalStrain(thermal_strain, instantaneous_cte);

    _deigenstrain_dT[_qp].zero();
    _deigenstrain_dT[_qp].addIa(instantaneous_cte);
  }

  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(thermal_strain);
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain(
    Real & /*thermal_strain*/, Real & /*instantaneous_cte*/)
{
  if constexpr (is_ad)
    mooseError("This method should never get called");
  else
    mooseError("You must override ComputeThermalExpansionEigenstrainBase::computeThermalStrain");
}

template <bool is_ad>
void
ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::computeThermalStrain(
    ADReal & /*thermal_strain*/)
{
  if constexpr (is_ad)
    mooseError("You must override ADComputeThermalExpansionEigenstrainBase::computeThermalStrain");
  else
    mooseError("This method should never get called");
}

template class ComputeThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeThermalExpansionEigenstrainBaseTempl<true>;
