#include "PlenumPressure.h"

template<>
InputParameters validParams<PlenumPressure>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<int>("component", "The component for the PlenumPressure");
  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::string>("material_input", "", "The name of the postprocessor value that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<std::string>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<std::string>("volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>("startup_time", 0, "The amount of time during which the pressure will ramp from zero to its true value.");
  params.addParam<std::string>("output_initial_moles", "", "The reporting postprocessor to use for the initial moles of gas.");
  params.addParam<std::string>("output", "", "The reporting postprocessor to use for the plenum pressure value.");
  return params;
}

PlenumPressure::PlenumPressure(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _n0(0),
   _component(getParam<int>("component")),
   _initial_pressure(getParam<Real>("initial_pressure")),
   _material_input( getParam<std::string>("material_input") != "" ? &getPostprocessorValue(getParam<std::string>("material_input")) : NULL ),
   _R(getParam<Real>("R")),
   _temperature( getPostprocessorValue(getParam<std::string>("temperature"))),
   _volume( getPostprocessorValue(getParam<std::string>("volume"))),
   _startup_time( getParam<Real>("startup_time")),
   _initial_moles( getParam<std::string>("output_initial_moles") != "" ? &getPostprocessorValue(getParam<std::string>("output_initial_moles")) : NULL ),
   _output( getParam<std::string>("output") != "" ? &getPostprocessorValue(getParam<std::string>("output")) : NULL ),
   _my_value(0)
{
  if(_component < 0 || _component > 2 )
  {
    std::stringstream errMsg;
    errMsg << "Invalid component given for "
           << name
           << ": "
           << _component
           << "." << std::endl;

    mooseError( errMsg );
  }

}

Real
PlenumPressure::computeQpResidual()
{
  return _my_value * (_normals[_qp](_component) * _test[_i][_qp]);
}

void PlenumPressure::initialSetup()
{
  _n0 = _initial_pressure * _volume / (_R * _temperature);

  if ( _initial_moles )
  {
    *_initial_moles = _n0;
  }
  const Real factor = _t >= _startup_time ? 1.0 : _t / _startup_time;
  _my_value = factor * _initial_pressure;
  if (_output)
  {
    *_output = _my_value;
  }
}

void
PlenumPressure::residualSetup()
{
  Real n = _n0;
  if (_material_input)
    n += *_material_input;
  const Real pressure = n * _R * _temperature / _volume;

  const Real factor = _t >= _startup_time ? 1.0 : _t / _startup_time;
  _my_value = factor * pressure;
  if (_output)
  {
    *_output = _my_value;
  }
}
