//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstitutiveModel.h"
#include "Function.h"

template <>
InputParameters
validParams<ConstitutiveModel>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_expansion", "The thermal expansion coefficient.");
  params.addParam<FunctionName>("thermal_expansion_function",
                                "Thermal expansion coefficient as a function of temperature.");
  params.addParam<Real>(
      "stress_free_temperature",
      "The stress-free temperature.  If not specified, the initial temperature is used.");
  params.addParam<Real>("thermal_expansion_reference_temperature",
                        "Reference temperature for mean thermal expansion function.");
  MooseEnum cte_function_type("instantaneous mean");
  params.addParam<MooseEnum>("thermal_expansion_function_type",
                             cte_function_type,
                             "Type of thermal expansion function.  Choices are: " +
                                 cte_function_type.getRawNames());

  return params;
}

ConstitutiveModel::ConstitutiveModel(const InputParameters & parameters)
  : Material(parameters),
    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _temperature_old(_has_temp ? coupledValueOld("temp") : _zero),
    _alpha(parameters.isParamValid("thermal_expansion") ? getParam<Real>("thermal_expansion") : 0.),
    _alpha_function(parameters.isParamValid("thermal_expansion_function")
                        ? &getFunction("thermal_expansion_function")
                        : NULL),
    _has_stress_free_temp(isParamValid("stress_free_temperature")),
    _stress_free_temp(_has_stress_free_temp ? getParam<Real>("stress_free_temperature") : 0.0),
    _ref_temp(0.0),
    _step_zero_cm(declareRestartableData<bool>("step_zero_cm", true)),
    _step_one_cm(declareRestartableData<bool>("step_one_cm", true))
{
  if (parameters.isParamValid("thermal_expansion_function_type"))
  {
    if (!_alpha_function)
      mooseError("thermal_expansion_function_type can only be set when thermal_expansion_function "
                 "is used");
    MooseEnum tec = getParam<MooseEnum>("thermal_expansion_function_type");
    if (tec == "mean")
      _mean_alpha_function = true;
    else if (tec == "instantaneous")
      _mean_alpha_function = false;
    else
      mooseError("Invalid option for thermal_expansion_function_type");
  }
  else
    _mean_alpha_function = false;

  if (parameters.isParamValid("thermal_expansion_reference_temperature"))
  {
    if (!_alpha_function)
      mooseError("thermal_expansion_reference_temperature can only be set when "
                 "thermal_expansion_function is used");
    if (!_mean_alpha_function)
      mooseError("thermal_expansion_reference_temperature can only be set when "
                 "thermal_expansion_function_type = mean");
    _ref_temp = getParam<Real>("thermal_expansion_reference_temperature");
    if (!_has_temp)
      mooseError(
          "Cannot specify thermal_expansion_reference_temperature without coupling to temperature");
  }
  else if (_mean_alpha_function)
    mooseError("Must specify thermal_expansion_reference_temperature if "
               "thermal_expansion_function_type = mean");
}

void
ConstitutiveModel::setQp(unsigned int qp)
{
  _qp = qp;
}

void
ConstitutiveModel::computeStress(const Elem & /*current_elem*/,
                                 const SymmElasticityTensor & elasticityTensor,
                                 const SymmTensor & stress_old,
                                 SymmTensor & strain_increment,
                                 SymmTensor & stress_new)
{
  stress_new = elasticityTensor * strain_increment;
  stress_new += stress_old;
}

bool
ConstitutiveModel::applyThermalStrain(SymmTensor & strain_increment, SymmTensor & d_strain_dT)
{
  if (_t_step >= 1)
    _step_zero_cm = false;

  if (_t_step >= 2)
    _step_one_cm = false;

  if (_has_temp && !_step_zero_cm)
  {
    Real inc_thermal_strain;
    Real d_thermal_strain_d_temp;

    Real old_temp;
    if (_step_one_cm && _has_stress_free_temp)
      old_temp = _stress_free_temp;
    else
      old_temp = _temperature_old[_qp];

    Real current_temp = _temperature[_qp];

    Real delta_t = current_temp - old_temp;

    Real alpha = _alpha;

    if (_alpha_function)
    {
      Point p;
      Real alpha_current_temp = _alpha_function->value(current_temp, p);
      Real alpha_old_temp = _alpha_function->value(old_temp, p);
      Real alpha_stress_free_temperature = _alpha_function->value(_stress_free_temp, p);

      if (_mean_alpha_function)
      {
        Real small(1e-6);

        Real numerator = alpha_current_temp * (current_temp - _ref_temp) -
                         alpha_old_temp * (old_temp - _ref_temp);
        Real denominator = 1.0 + alpha_stress_free_temperature * (_stress_free_temp - _ref_temp);
        if (denominator < small)
          mooseError("Denominator too small in thermal strain calculation");
        inc_thermal_strain = numerator / denominator;
        d_thermal_strain_d_temp = alpha_current_temp * (current_temp - _ref_temp);
      }
      else
      {
        inc_thermal_strain = delta_t * 0.5 * (alpha_current_temp + alpha_old_temp);
        d_thermal_strain_d_temp = alpha_current_temp;
      }
    }
    else
    {
      inc_thermal_strain = delta_t * alpha;
      d_thermal_strain_d_temp = alpha;
    }

    strain_increment.addDiag(-inc_thermal_strain);
    d_strain_dT.addDiag(-d_thermal_strain_d_temp);
  }

  bool modified = true;
  return modified;
}
