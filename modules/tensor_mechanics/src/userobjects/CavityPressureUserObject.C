/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CavityPressureUserObject.h"

template <>
InputParameters
validParams<CavityPressureUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<Real>(
      "initial_pressure",
      0,
      "The initial pressure in the cavity.  If not given, a zero initial pressure will be used.");
  params.addParam<std::vector<PostprocessorName>>("material_input",
                                                  "The name of the postprocessor(s) that holds the "
                                                  "amount of material injected into the cavity.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<PostprocessorName>(
      "temperature", "The name of the average temperature postprocessor value.");
  params.addParam<Real>("initial_temperature", "Initial temperature (optional)");
  params.addRequiredParam<PostprocessorName>(
      "volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>(
      "startup_time",
      0,
      "The amount of time during which the pressure will ramp from zero to its true value.");
  params.set<bool>("use_displaced_mesh") = true;

  params.addPrivateParam<std::string>("built_by_action", ""); // Hide from input file dump
  return params;
}

CavityPressureUserObject::CavityPressureUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    _cavity_pressure(declareRestartableData<Real>("cavity_pressure", 0)),
    _n0(declareRestartableData<Real>("initial_moles", 0)),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _material_input(),
    _R(getParam<Real>("R")),
    _temperature(getPostprocessorValue("temperature")),
    _init_temp_given(isParamValid("initial_temperature")),
    _init_temp(_init_temp_given ? getParam<Real>("initial_temperature") : 0),
    _volume(getPostprocessorValue("volume")),
    _start_time(0),
    _startup_time(getParam<Real>("startup_time")),
    _initialized(declareRestartableData<bool>("initialized", false))
{
  if (isParamValid("material_input"))
  {
    std::vector<PostprocessorName> ppn =
        params.get<std::vector<PostprocessorName>>("material_input");
    const unsigned int len = ppn.size();
    for (unsigned int i = 0; i < len; ++i)
      _material_input.push_back(&getPostprocessorValueByName(ppn[i]));
  }
}

Real
CavityPressureUserObject::getValue(const std::string & quantity) const
{
  Real value = 0;
  if ("initial_moles" == quantity)
    value = _n0;
  else if ("cavity_pressure" == quantity)
    value = _cavity_pressure;
  else
    mooseError("Unknown quantity in " + name());

  return value;
}

void
CavityPressureUserObject::initialize()
{
  if (!_initialized)
  {
    Real init_temp = _temperature;
    if (_init_temp_given)
      init_temp = _init_temp;

    if (MooseUtils::absoluteFuzzyLessEqual(init_temp, 0.0))
      mooseError("Cannot have initial temperature of zero when initializing cavity pressure. "
                 "Does the supplied Postprocessor for temperature execute at initial?");

    _n0 = _initial_pressure * _volume / (_R * init_temp);
    _start_time = _t - _dt;
    const Real factor =
        _t >= _start_time + _startup_time ? 1.0 : (_t - _start_time) / _startup_time;
    _cavity_pressure = factor * _initial_pressure;
    _initialized = true;
  }
}

void
CavityPressureUserObject::execute()
{
  Real mat = 0;
  for (unsigned int i = 0; i < _material_input.size(); ++i)
    mat += *_material_input[i];

  const Real pressure = (_n0 + mat) * _R * _temperature / _volume;
  const Real factor = _t >= _start_time + _startup_time ? 1.0 : (_t - _start_time) / _startup_time;
  _cavity_pressure = factor * pressure;
}
