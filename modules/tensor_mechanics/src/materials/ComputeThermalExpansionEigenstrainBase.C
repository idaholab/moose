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
    _temperature(this->template coupledGenericValue<is_ad>("temperature")),
    _temperature_old(this->_fe_problem.isTransient() ? this->coupledValueOld("temperature")
                                                     : this->_zero),
    _deigenstrain_dT(is_ad ? nullptr
                           : &this->template declarePropertyDerivative<RankTwoTensor>(
                                 _eigenstrain_name, this->getVar("temperature", 0)->name())),
    _stress_free_temperature(this->coupledValue("stress_free_temperature"))
{
  if (_use_old_temperature && !this->_fe_problem.isTransient())
    this->paramError(
        "use_old_temperature",
        "The old state of the temperature variable is only available in a transient simulation.");
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
    // dthermal_strain_dT is just the derivative of thermal_strain with respect to temperature
    Real dthermal_strain_dT = 0.0;
    computeThermalStrain(thermal_strain, &dthermal_strain_dT);

    (*_deigenstrain_dT)[_qp].zero();
    (*_deigenstrain_dT)[_qp].addIa(dthermal_strain_dT);
  }

  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(thermal_strain);
}

template class ComputeThermalExpansionEigenstrainBaseTempl<false>;
template class ComputeThermalExpansionEigenstrainBaseTempl<true>;
