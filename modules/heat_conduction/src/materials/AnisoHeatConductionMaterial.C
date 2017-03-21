/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AnisoHeatConductionMaterial.h"
#include "Function.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<AnisoHeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity_x", "The thermal conductivity in the x direction");
  params.addParam<Real>("thermal_conductivity_y", "The thermal conductivity in the y direction");
  params.addParam<Real>("thermal_conductivity_z", "The thermal conductivity in the z direction");
  params.addParam<PostprocessorName>("thermal_conductivity_x_pp",
                                     "The thermal conductivity PP name in the x direction");
  params.addParam<PostprocessorName>("thermal_conductivity_y_pp",
                                     "The thermal conductivity PP name in the y direction");
  params.addParam<PostprocessorName>("thermal_conductivity_z_pp",
                                     "The thermal conductivity PP name in the z direction");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>(
      "specific_heat_temperature_function", "", "Specific heat as a function of temperature.");

  return params;
}

AnisoHeatConductionMaterial::AnisoHeatConductionMaterial(const InputParameters & parameters)
  : Material(parameters),

    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),

    _my_thermal_conductivity_x(
        isParamValid("thermal_conductivity_x") ? getParam<Real>("thermal_conductivity_x") : -1),
    _my_thermal_conductivity_y(
        isParamValid("thermal_conductivity_y") ? getParam<Real>("thermal_conductivity_y") : -1),
    _my_thermal_conductivity_z(
        isParamValid("thermal_conductivity_z") ? getParam<Real>("thermal_conductivity_z") : -1),

    _thermal_conductivity_x_pp(isParamValid("thermal_conductivity_x_pp")
                                   ? &getPostprocessorValue("thermal_conductivity_x_pp")
                                   : NULL),
    _thermal_conductivity_y_pp(isParamValid("thermal_conductivity_y_pp")
                                   ? &getPostprocessorValue("thermal_conductivity_y_pp")
                                   : NULL),
    _thermal_conductivity_z_pp(isParamValid("thermal_conductivity_z_pp")
                                   ? &getPostprocessorValue("thermal_conductivity_z_pp")
                                   : NULL),

    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),

    _thermal_conductivity_x(&declareProperty<Real>("thermal_conductivity_x")),
    _thermal_conductivity_x_dT(&declareProperty<Real>("thermal_conductivity_x_dT")),
    _thermal_conductivity_y(isParamValid("thermal_conductivity_y") ||
                                    isParamValid("thermal_conductivity_y_pp")
                                ? &declareProperty<Real>("thermal_conductivity_y")
                                : NULL),
    _thermal_conductivity_y_dT(
        _thermal_conductivity_y ? &declareProperty<Real>("thermal_conductivity_y_dT") : NULL),
    _thermal_conductivity_z(isParamValid("thermal_conductivity_z") ||
                                    isParamValid("thermal_conductivity_z_pp")
                                ? &declareProperty<Real>("thermal_conductivity_z")
                                : NULL),
    _thermal_conductivity_z_dT(
        _thermal_conductivity_z ? &declareProperty<Real>("thermal_conductivity_z_dT") : NULL),

    _specific_heat(declareProperty<Real>("specific_heat")),
    _specific_heat_temperature_function(
        getParam<FunctionName>("specific_heat_temperature_function") != ""
            ? &getFunction("specific_heat_temperature_function")
            : NULL)
{
  bool k_x = isParamValid("thermal_conductivity_x") || (NULL != _thermal_conductivity_x_pp);
  bool k_y = isParamValid("thermal_conductivity_y") || (NULL != _thermal_conductivity_y_pp);
  bool k_z = isParamValid("thermal_conductivity_z") || (NULL != _thermal_conductivity_z_pp);

  if (!k_x || (_subproblem.mesh().dimension() > 1 && !k_y) ||
      (_subproblem.mesh().dimension() > 2 && !k_z))
  {
    mooseError("Incomplete set of orthotropic thermal conductivity parameters");
  }
  if (_specific_heat_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using specific heat function");
  }
  if (isParamValid("specific_heat") && _specific_heat_temperature_function)
  {
    mooseError("Cannot define both specific heat and specific heat temperature function");
  }

  k_x = isParamValid("thermal_conductivity_x") && (NULL != _thermal_conductivity_x_pp);
  k_y = isParamValid("thermal_conductivity_y") && (NULL != _thermal_conductivity_y_pp);
  k_z = isParamValid("thermal_conductivity_z") && (NULL != _thermal_conductivity_z_pp);
  if (k_x || k_y || k_z)
  {
    mooseError("Cannot define thermal conductivity value and Postprocessor");
  }
}

void
AnisoHeatConductionMaterial::computeProperties()
{
  for (unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    (*_thermal_conductivity_x)[qp] =
        _thermal_conductivity_x_pp ? *_thermal_conductivity_x_pp : _my_thermal_conductivity_x;
    (*_thermal_conductivity_x_dT)[qp] = 0;
    if (_thermal_conductivity_y)
    {
      (*_thermal_conductivity_y)[qp] =
          _thermal_conductivity_y_pp ? *_thermal_conductivity_y_pp : _my_thermal_conductivity_y;
      (*_thermal_conductivity_y_dT)[qp] = 0;
    }
    if (_thermal_conductivity_z)
    {
      (*_thermal_conductivity_z)[qp] =
          _thermal_conductivity_z_pp ? *_thermal_conductivity_z_pp : _my_thermal_conductivity_z;
      (*_thermal_conductivity_z_dT)[qp] = 0;
    }

    if (_specific_heat_temperature_function)
    {
      Point p;
      _specific_heat[qp] = _specific_heat_temperature_function->value(_temperature[qp], p);
    }
    else
    {
      _specific_heat[qp] = _my_specific_heat;
    }
  }
}
