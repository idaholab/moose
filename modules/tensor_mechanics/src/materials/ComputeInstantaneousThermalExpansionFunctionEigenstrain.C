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
  return params;
}

template <bool is_ad>
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::
    ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBaseTempl<is_ad>(parameters),
    _thermal_expansion_function(this->getFunction("thermal_expansion_function")),
    _thermal_strain(this->template declareProperty<Real>(
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
void
ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<is_ad>::computeThermalStrain(
    GenericReal<is_ad> & thermal_strain, Real * dthermal_strain_dT)
{
  if (this->_t_step > 1)
    _step_one = false;

  const auto & current_temp = this->_temperature[_qp];

  const Real & old_thermal_strain = _thermal_strain_old[_qp];

  const auto & old_temp =
      (_step_one ? this->_stress_free_temperature[_qp] : this->_temperature_old[_qp]);
  const auto delta_T = current_temp - old_temp;

  const auto alpha_current_temp = _thermal_expansion_function.value(current_temp);
  const auto alpha_old_temp = _thermal_expansion_function.value(old_temp);

  thermal_strain = old_thermal_strain + delta_T * 0.5 * (alpha_current_temp + alpha_old_temp);

  // store this for use in the next timestep (no derivatives needed)
  _thermal_strain[_qp] = MetaPhysicL::raw_value(thermal_strain);

  if constexpr (!is_ad)
  {
    mooseAssert(dthermal_strain_dT, "Internal error. dthermal_strain_dT should not be nullptr.");
    *dthermal_strain_dT = alpha_current_temp;
  }
  else
    libmesh_ignore(dthermal_strain_dT);
}

template class ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<false>;
template class ComputeInstantaneousThermalExpansionFunctionEigenstrainTempl<true>;
