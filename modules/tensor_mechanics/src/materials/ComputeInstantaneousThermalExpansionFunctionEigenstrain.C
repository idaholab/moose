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

InputParameters
ComputeInstantaneousThermalExpansionFunctionEigenstrain::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the instantaneous thermal expansion as a function of "
                             "temperature");
  params.addRequiredParam<FunctionName>("thermal_expansion_function",
                                        "Function describing the instantaneous thermal expansion "
                                        "coefficient as a function of temperature");
  return params;
}

ComputeInstantaneousThermalExpansionFunctionEigenstrain::
    ComputeInstantaneousThermalExpansionFunctionEigenstrain(const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBase(parameters),
    _temperature_old(coupledValueOld("temperature")),
    _thermal_expansion_function(getFunction("thermal_expansion_function")),
    _thermal_strain(
        declareProperty<Real>(_base_name + "InstantaneousThermalExpansionFunction_thermal_strain")),
    _thermal_strain_old(getMaterialPropertyOld<Real>(
        _base_name + "InstantaneousThermalExpansionFunction_thermal_strain")),
    _step_one(declareRestartableData<bool>("step_one", true))
{
}

void
ComputeInstantaneousThermalExpansionFunctionEigenstrain::initQpStatefulProperties()
{
  _thermal_strain[_qp] = 0;
}

void
ComputeInstantaneousThermalExpansionFunctionEigenstrain::computeThermalStrain(
    Real & thermal_strain, Real & instantaneous_cte)
{
  if (_t_step > 1)
    _step_one = false;

  const Real & current_temp = _temperature[_qp];

  const Real & old_thermal_strain = _thermal_strain_old[_qp];

  const Real & old_temp = (_step_one ? _stress_free_temperature[_qp] : _temperature_old[_qp]);
  const Real delta_T = current_temp - old_temp;

  const Point p;
  const Real alpha_current_temp = _thermal_expansion_function.value(current_temp, p);
  const Real alpha_old_temp = _thermal_expansion_function.value(old_temp, p);

  thermal_strain = old_thermal_strain + delta_T * 0.5 * (alpha_current_temp + alpha_old_temp);
  _thermal_strain[_qp] = thermal_strain;

  instantaneous_cte = alpha_current_temp;
}
