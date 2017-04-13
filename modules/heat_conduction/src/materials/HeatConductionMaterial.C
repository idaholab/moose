/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "HeatConductionMaterial.h"
#include "Function.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<HeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addParam<FunctionName>("thermal_conductivity_temperature_function",
                                "",
                                "Thermal conductivity as a function of temperature.");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>(
      "specific_heat_temperature_function", "", "Specific heat as a function of temperature.");

  return params;
}

HeatConductionMaterial::HeatConductionMaterial(const InputParameters & parameters)
  : Material(parameters),

    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _my_thermal_conductivity(
        isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),

    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(declareProperty<Real>("thermal_conductivity_dT")),
    _thermal_conductivity_temperature_function(
        getParam<FunctionName>("thermal_conductivity_temperature_function") != ""
            ? &getFunction("thermal_conductivity_temperature_function")
            : NULL),

    _specific_heat(declareProperty<Real>("specific_heat")),
    _specific_heat_temperature_function(
        getParam<FunctionName>("specific_heat_temperature_function") != ""
            ? &getFunction("specific_heat_temperature_function")
            : NULL)
{
  if (_thermal_conductivity_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using thermal conductivity function");
  }
  if (isParamValid("thermal_conductivity") && _thermal_conductivity_temperature_function)
  {
    mooseError(
        "Cannot define both thermal conductivity and thermal conductivity temperature function");
  }
  if (_specific_heat_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using specific heat function");
  }
  if (isParamValid("specific_heat") && _specific_heat_temperature_function)
  {
    mooseError("Cannot define both specific heat and specific heat temperature function");
  }
}

void
HeatConductionMaterial::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    Real qp_temperature = 0;
    if (_has_temp)
    {
      qp_temperature = _temperature[qp];
      if (_temperature[qp] < 0)
      {
        std::stringstream msg;
        msg << "WARNING:  In HeatConductionMaterial:  negative temperature!\n"
            << "\tResetting to zero.\n"
            << "\t_qp: " << qp << "\n"
            << "\ttemp: " << _temperature[qp] << "\n"
            << "\telem: " << _current_elem->id() << "\n"
            << "\tproc: " << processor_id() << "\n";
        mooseWarning(msg.str());
        qp_temperature = 0;
      }
    }
    if (_thermal_conductivity_temperature_function)
    {
      Point p;
      _thermal_conductivity[qp] =
          _thermal_conductivity_temperature_function->value(qp_temperature, p);
      _thermal_conductivity_dT[qp] =
          _thermal_conductivity_temperature_function->timeDerivative(qp_temperature, p);
    }
    else
    {
      _thermal_conductivity[qp] = _my_thermal_conductivity;
      _thermal_conductivity_dT[qp] = 0;
    }

    if (_specific_heat_temperature_function)
    {
      Point p;
      _specific_heat[qp] = _specific_heat_temperature_function->value(qp_temperature, p);
    }
    else
    {
      _specific_heat[qp] = _my_specific_heat;
    }
  }
}
