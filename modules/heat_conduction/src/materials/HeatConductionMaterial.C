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
    _temperature((_has_temp && !is_ad) ? coupledValue("temp") : _zero),
    _ad_temperature((_has_temp && is_ad) ? adCoupledValue("temp") : _ad_zero),
    _my_thermal_conductivity(
        isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),

    _thermal_conductivity(declareGenericProperty<Real, is_ad>("thermal_conductivity")),
    _thermal_conductivity_dT(declareProperty<Real>("thermal_conductivity_dT")),
    _thermal_conductivity_temperature_function(
        getParam<FunctionName>("thermal_conductivity_temperature_function") != ""
            ? &getFunction("thermal_conductivity_temperature_function")
            : NULL),

    _specific_heat(declareGenericProperty<Real, is_ad>("specific_heat")),
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

template <bool is_ad>
void
HeatConductionMaterialTempl<is_ad>::setDerivatives(GenericReal<is_ad> & prop,
                                                   Real dprop_dT,
                                                   const ADReal & ad_T)
{
  if (ad_T < 0)
    prop.derivatives() = 0;
  else
    prop.derivatives() = dprop_dT * ad_T.derivatives();
}

template <>
void
HeatConductionMaterialTempl<false>::setDerivatives(Real &, Real, const ADReal &)
{
  mooseError("Mistaken call of setDerivatives in a non-AD HeatConductionMaterial version");
}

template <bool is_ad>
void
HeatConductionMaterialTempl<is_ad>::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    Real qp_temperature = 0;
    if (_has_temp)
    {
      if (is_ad)
        qp_temperature = MetaPhysicL::raw_value(_ad_temperature[qp]);
      else
        qp_temperature = _temperature[qp];
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
      Point p;
      _thermal_conductivity[qp] =
          _thermal_conductivity_temperature_function->value(qp_temperature, p);
      // A terrible exploitation of the Function API to get a derivative with respect to a
      // non-linear variable
      _thermal_conductivity_dT[qp] =
          _thermal_conductivity_temperature_function->timeDerivative(qp_temperature, p);
      if (is_ad)
        setDerivatives(
            _thermal_conductivity[qp], _thermal_conductivity_dT[qp], _ad_temperature[qp]);
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
      if (is_ad)
      {
        // A terrible exploitation of the Function API to get a derivative with respect to a
        // non-linear variable
        Real dcp_dT = _specific_heat_temperature_function->timeDerivative(qp_temperature, p);
        setDerivatives(_specific_heat[qp], dcp_dT, _ad_temperature[qp]);
      }
    }
    else
    {
      _specific_heat[qp] = _my_specific_heat;
    }
  }
}

template class HeatConductionMaterialTempl<false>;
template class HeatConductionMaterialTempl<true>;
