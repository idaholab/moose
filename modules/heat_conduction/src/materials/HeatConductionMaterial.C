//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionMaterial.h"
#include "Function.h"

#include "libmesh/quadrature.h"

registerMooseObject("HeatConductionApp", HeatConductionMaterial);
registerMooseObject("HeatConductionApp", ADHeatConductionMaterial);

template <bool is_ad>
InputParameters
HeatConductionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addParam<FunctionName>("thermal_conductivity_temperature_function",
                                "",
                                "Thermal conductivity as a function of temperature.");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>(
      "specific_heat_temperature_function", "", "Specific heat as a function of temperature.");
  params.addClassDescription("General-purpose material model for heat conduction");

  return params;
}

template <bool is_ad>
HeatConductionMaterialTempl<is_ad>::HeatConductionMaterialTempl(const InputParameters & parameters)
  : Material(parameters),
    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledGenericValue<is_ad>("temp") : genericZeroValue<is_ad>()),
    _my_thermal_conductivity(
        isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),

    _thermal_conductivity(declareGenericProperty<Real, is_ad>("thermal_conductivity")),
    _thermal_conductivity_dT(declareProperty<Real>("thermal_conductivity_dT")),
    _thermal_conductivity_temperature_function(
        getParam<FunctionName>("thermal_conductivity_temperature_function") != ""
            ? &getFunction("thermal_conductivity_temperature_function")
            : nullptr),

    _specific_heat(declareGenericProperty<Real, is_ad>("specific_heat")),
    _specific_heat_temperature_function(
        getParam<FunctionName>("specific_heat_temperature_function") != ""
            ? &getFunction("specific_heat_temperature_function")
            : nullptr)
{
  if (_thermal_conductivity_temperature_function && !_has_temp)
    paramError("thermal_conductivity_temperature_function",
               "Must couple with temperature if using thermal conductivity function");

  if (isParamValid("thermal_conductivity") && _thermal_conductivity_temperature_function)
    mooseError(
        "Cannot define both thermal conductivity and thermal conductivity temperature function");

  if (_specific_heat_temperature_function && !_has_temp)
    paramError("specific_heat_temperature_function",
               "Must couple with temperature if using specific heat function");

  if (isParamValid("specific_heat") && _specific_heat_temperature_function)
    mooseError("Cannot define both specific heat and specific heat temperature function");
}

template <bool is_ad>
void
HeatConductionMaterialTempl<is_ad>::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    auto qp_temperature = _temperature[qp];
    if (_has_temp)
    {
      if (qp_temperature < 0)
      {
        std::stringstream msg;
        msg << "WARNING:  In HeatConductionMaterial:  negative temperature!\n"
            << "\tResetting to zero.\n"
            << "\t_qp: " << qp << "\n"
            << "\ttemp: " << qp_temperature << "\n"
            << "\telem: " << _current_elem->id() << "\n"
            << "\tproc: " << processor_id() << "\n";
        mooseWarning(msg.str());
        qp_temperature = 0;
      }
    }
    if (_thermal_conductivity_temperature_function)
    {
      _thermal_conductivity[qp] = _thermal_conductivity_temperature_function->value(qp_temperature);
      _thermal_conductivity_dT[qp] = _thermal_conductivity_temperature_function->timeDerivative(
          MetaPhysicL::raw_value(qp_temperature));
    }
    else
    {
      _thermal_conductivity[qp] = _my_thermal_conductivity;
      _thermal_conductivity_dT[qp] = 0;
    }

    if (_specific_heat_temperature_function)
      _specific_heat[qp] = _specific_heat_temperature_function->value(qp_temperature);
    else
      _specific_heat[qp] = _my_specific_heat;
  }
}

template class HeatConductionMaterialTempl<false>;
template class HeatConductionMaterialTempl<true>;
