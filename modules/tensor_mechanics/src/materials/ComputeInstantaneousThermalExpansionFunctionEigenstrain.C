//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeInstantaneousThermalExpansionFunctionEigenstrain.h"
#include "Function.h"
#include "CastDualNumber.h"
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeInstantaneousThermalExpansionFunctionEigenstrain);
registerMooseObject("TensorMechanicsApp",
                    ADComputeInstantaneousThermalExpansionFunctionEigenstrain);

template <bool is_ad>
InputParameters
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the instantaneous thermal expansion as a function of "
                             "temperature");
  params.addRequiredParam<FunctionName>("thermal_expansion_function",
                                        "Function describing the instantaneous thermal expansion "
                                        "coefficient as a function of temperature");
  params.suppressParameter<bool>("use_old_temperature");
  return params;
}

template <bool is_ad>
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::
    ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _thermal_expansion_function(this->getFunction("thermal_expansion_function")),
    _thermal_strain(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name + "InstantaneousThermalExpansionFunction_thermal_strain")),
    _thermal_strain_old(this->template getMaterialPropertyOld<Real>(
        this->_base_name + "InstantaneousThermalExpansionFunction_thermal_strain")),
    _step_one(this->template declareRestartableData<bool>("step_one", true))
{
  if (this->_use_old_temperature)
    this->paramError("use_old_temperature",
                     "The old temperature value cannot be used in this incremental update model.");
}

template <bool is_ad>
void
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::initQpStatefulProperties()
{
  _thermal_strain[_qp] = 0;
}

template <bool is_ad>
ValueAndDerivative<is_ad>
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::computeThermalStrain()
{
  if (this->_t_step > 1)
    _step_one = false;

  const auto & old_temp =
      (_step_one ? this->_stress_free_temperature[_qp] : this->_temperature_old[_qp]);
  const auto delta_T = this->_temperature[_qp] - old_temp;

  const auto alpha_current_temp = _thermal_expansion_function.value(this->_temperature[_qp]);
  const auto alpha_old_temp = _thermal_expansion_function.value(old_temp);

  const auto thermal_strain =
      _thermal_strain_old[_qp] + delta_T * 0.5 * (alpha_current_temp + alpha_old_temp);

  // Store thermal strain for use in the next timestep (casts ValueAndDerivative<is_ad>
  // to GenericReal<is_ad>).
  _thermal_strain[_qp] = dual_number_cast<GenericReal<is_ad>>(thermal_strain);

  return thermal_strain;
}

template class ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<false>;
template class ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<true>;
