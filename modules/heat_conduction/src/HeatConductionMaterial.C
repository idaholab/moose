#include "HeatConductionMaterial.h"

template<>
InputParameters validParams<HeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addParam<PostprocessorName>("thermal_conductivity_x", "The thermal conductivity PP name in the x direction");
  params.addParam<PostprocessorName>("thermal_conductivity_y", "The thermal conductivity PP name in the y direction");
  params.addParam<PostprocessorName>("thermal_conductivity_z", "The thermal conductivity PP name in the z direction");
  params.addParam<FunctionName>("thermal_conductivity_temperature_function", "", "Thermal conductivity as a function of temperature.");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>("specific_heat_temperature_function", "", "Specific heat as a function of temperature.");

  return params;
}

HeatConductionMaterial::HeatConductionMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),

    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? coupledValue("temp") : _zero),
    _my_thermal_conductivity(isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _my_thermal_conductivity_x(isParamValid("thermal_conductivity_x") ? &getPostprocessorValue("thermal_conductivity_x") : NULL),
    _my_thermal_conductivity_y(isParamValid("thermal_conductivity_y") ? &getPostprocessorValue("thermal_conductivity_y") : NULL),
    _my_thermal_conductivity_z(isParamValid("thermal_conductivity_z") ? &getPostprocessorValue("thermal_conductivity_z") : NULL),
    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),
    _isotropic_thcond(isParamValid("thermal_conductivity") || getParam<FunctionName>("thermal_conductivity_temperature_function") != ""),

    _thermal_conductivity(_isotropic_thcond ? &declareProperty<Real>("thermal_conductivity") : NULL),
    _thermal_conductivity_dT(_isotropic_thcond ? &declareProperty<Real>("thermal_conductivity_dT") : NULL),
    _thermal_conductivity_x(isParamValid("thermal_conductivity_x") ? &declareProperty<Real>("thermal_conductivity_x") : NULL),
    _thermal_conductivity_x_dT(isParamValid("thermal_conductivity_x") ? &declareProperty<Real>("thermal_conductivity_x_dT") : NULL),
    _thermal_conductivity_y(isParamValid("thermal_conductivity_y") ? &declareProperty<Real>("thermal_conductivity_y") : NULL),
    _thermal_conductivity_y_dT(isParamValid("thermal_conductivity_y") ? &declareProperty<Real>("thermal_conductivity_y_dT") : NULL),
    _thermal_conductivity_z(isParamValid("thermal_conductivity_z") ? &declareProperty<Real>("thermal_conductivity_z") : NULL),
    _thermal_conductivity_z_dT(isParamValid("thermal_conductivity_z") ? &declareProperty<Real>("thermal_conductivity_z_dT") : NULL),
    _thermal_conductivity_temperature_function( getParam<FunctionName>("thermal_conductivity_temperature_function") != "" ? &getFunction("thermal_conductivity_temperature_function") : NULL),
    _specific_heat(declareProperty<Real>("specific_heat")),
    _specific_heat_temperature_function( getParam<FunctionName>("specific_heat_temperature_function") != "" ? &getFunction("specific_heat_temperature_function") : NULL)
{
  if (_thermal_conductivity_temperature_function && !_has_temp)
  {
    mooseError("Must couple with temperature if using thermal conductivity function");
  }
  if (_isotropic_thcond && (_thermal_conductivity_x || _thermal_conductivity_y || _thermal_conductivity_z))
  {
    mooseError("Cannot define both isotropic and orthotropic thermal conductivity");
  }
  if (!_isotropic_thcond && !_thermal_conductivity_x)
  {
    mooseError("Must define either isotropic or orthotropic thermal conductivity");
  }
  if (isParamValid("thermal_conductivity") && _thermal_conductivity_temperature_function)
  {
    mooseError("Cannot define both thermal conductivity and thermal conductivity temperature function");
  }
  if (!_isotropic_thcond &&
      ((_subproblem.mesh().dimension() > 1 && !_thermal_conductivity_y) ||
       (_subproblem.mesh().dimension() > 2 && !_thermal_conductivity_z)))
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
}

void
HeatConductionMaterial::computeProperties()
{
  for(unsigned int qp(0); qp < _qrule->n_points(); ++qp)
  {
    if (_isotropic_thcond)
    {
      if (_thermal_conductivity_temperature_function)
      {
        Point p;
        (*_thermal_conductivity)[qp] = _thermal_conductivity_temperature_function->value(_temperature[qp], p);
        (*_thermal_conductivity_dT)[qp] = 0;
      }
      else
      {
        (*_thermal_conductivity)[qp] = _my_thermal_conductivity;
        (*_thermal_conductivity_dT)[qp] = 0;
      }
    }
    else
    {
      (*_thermal_conductivity_x)[qp] = *_my_thermal_conductivity_x;
      (*_thermal_conductivity_x_dT)[qp] = 0;

      if (_thermal_conductivity_y)
      {
        (*_thermal_conductivity_y)[qp] = *_my_thermal_conductivity_y;
        (*_thermal_conductivity_y_dT)[qp] = 0;
      }
      if (_thermal_conductivity_z)
      {
        (*_thermal_conductivity_z)[qp] = *_my_thermal_conductivity_z;
        (*_thermal_conductivity_z_dT)[qp] = 0;
      }
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
