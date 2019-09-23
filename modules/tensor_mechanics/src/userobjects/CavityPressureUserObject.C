//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressureUserObject.h"

registerMooseObject("TensorMechanicsApp", CavityPressureUserObject);

template <>
InputParameters
validParams<CavityPressureUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Uses the ideal gas law to compute internal pressure "
                             "and an initial moles of gas quantity.");
  params.addRangeCheckedParam<Real>(
      "initial_pressure",
      0.0,
      "initial_pressure >= 0.0",
      "The initial pressure in the cavity.  If not given, a zero initial pressure will be used.");
  params.addParam<std::vector<PostprocessorName>>("material_input",
                                                  "The name of the postprocessor(s) that holds the "
                                                  "amount of material injected into the cavity.");
  params.addRequiredRangeCheckedParam<Real>(
      "R", "R > 0.0", "The universal gas constant for the units used.");
  params.addRequiredParam<PostprocessorName>(
      "temperature", "The name of the average temperature postprocessor value.");
  params.addRangeCheckedParam<Real>(
      "initial_temperature", "initial_temperature > 0.0", "Initial temperature (optional)");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "volume",
      "The name of the postprocessor(s) that holds the value of the internal volume in the cavity");
  params.addParam<Real>(
      "startup_time",
      0.0,
      "The amount of time during which the pressure will ramp from zero to its true value.");
  params.set<bool>("use_displaced_mesh") = true;

  return params;
}

CavityPressureUserObject::CavityPressureUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    _cavity_pressure(declareRestartableData<Real>("cavity_pressure", 0.0)),
    _n0(declareRestartableData<Real>("initial_moles", 0.0)),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _material_input(params.get<std::vector<PostprocessorName>>("material_input").size()),
    _volume(params.get<std::vector<PostprocessorName>>("volume").size()),
    _R(getParam<Real>("R")),
    _temperature(getPostprocessorValue("temperature")),
    _init_temp_given(isParamValid("initial_temperature")),
    _init_temp(_init_temp_given ? getParam<Real>("initial_temperature") : 0.0),
    _startup_time(getParam<Real>("startup_time")),
    _initialized(declareRestartableData<bool>("initialized", false)),
    _start_time(0.0)
{
  auto material_names = params.get<std::vector<PostprocessorName>>("material_input");
  for (unsigned int i = 0; i < _material_input.size(); ++i)
    _material_input[i] = &getPostprocessorValueByName(material_names[i]);

  auto volume_names = params.get<std::vector<PostprocessorName>>("volume");
  for (unsigned int i = 0; i < volume_names.size(); ++i)
    _volume[i] = &getPostprocessorValueByName(volume_names[i]);
}

Real
CavityPressureUserObject::getValue(const MooseEnum & quantity) const
{
  Real value = 0;
  if (quantity == INITIAL_MOLES)
  {
    if (_n0 < 0.0)
      mooseError("In ",
                 _name,
                 ": Negative number of moles calculated as an input for the cavity pressure");

    value = _n0;
  }
  else if (quantity == CAVITY_PRESSURE)
    value = _cavity_pressure;
  else
    mooseError("In ", _name, ": Unknown quantity.");

  return value;
}

void
CavityPressureUserObject::initialize()
{
  if (!_initialized)
  {
    const Real volume = computeCavityVolume();
    Real init_temp = _temperature;
    if (_init_temp_given)
      init_temp = _init_temp;

    if (MooseUtils::absoluteFuzzyLessEqual(init_temp, 0.0))
      mooseError("Cannot have initial temperature of zero when initializing cavity pressure. "
                 "Does the supplied Postprocessor for temperature execute at initial?");

    _n0 = _initial_pressure * volume / (_R * init_temp);
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
  const Real volume = computeCavityVolume();
  Real mat = 0;

  for (unsigned int i = 0; i < _material_input.size(); ++i)
    mat += *_material_input[i];

  const Real pressure = (_n0 + mat) * _R * _temperature / volume;
  const Real factor = _t >= _start_time + _startup_time ? 1.0 : (_t - _start_time) / _startup_time;
  _cavity_pressure = factor * pressure;
}

Real
CavityPressureUserObject::computeCavityVolume()
{
  Real volume = 0;
  for (unsigned int i = 0; i < _volume.size(); ++i)
    volume += *_volume[i];

  return volume;
}
