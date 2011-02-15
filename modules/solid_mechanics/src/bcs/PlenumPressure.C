#include "PlenumPressure.h"

#include "MooseSystem.h"

template<>
InputParameters validParams<PlenumPressure>()
{
  InputParameters params = validParams<BoundaryCondition>();
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
  :BoundaryCondition(name, parameters),
   _initialized(false),
   _n0(0),
   _component(getParam<int>("component")),
   _initial_pressure(getParam<Real>("initial_pressure")),
   _material_input( getPostprocessorValue(getParam<std::string>("material_input")) ),
   _R(getParam<Real>("R")),
   _temperature( getPostprocessorValue(getParam<std::string>("temperature"))),
   _volume( getPostprocessorValue(getParam<std::string>("volume"))),
   _startup_time( getParam<Real>("startup_time")),
   _initial_He( getParam<std::string>("output_initial_moles") != "" ? &getPostprocessorValue(getParam<std::string>("output_initial_moles")) : NULL ),
   _output( getParam<std::string>("output") != "" ? &getPostprocessorValue(getParam<std::string>("output")) : NULL ),
   _my_value(0)
{
  if(_component < 0)
  {
    std::cout << "Must select a component for "
              << name
              << "." << std::endl;
    libmesh_error();
  }
  else if ( _component > 2 )
  {

    std::cout << "Invalid component given ("
              << _component
              << ") for "
              << name
              << "." << std::endl;

    libmesh_error();
  }

}

Real
PlenumPressure::computeQpResidual()
{
  return _my_value * (_normals[_qp](_component) * _phi[_i][_qp]);
}

void
PlenumPressure::setup()
{
  if ( _initialized )
  {
    const Real pressure = (_n0 + _material_input) * _R * _temperature / _volume;
    const Real factor = _t >= _startup_time ? 1.0 : _t / _startup_time;
    _my_value = factor * pressure;
    if (_output)
    {
      *_output = _my_value;
    }
  }
  else
  {
    _initialized = true;
    _n0 = _initial_pressure * _volume / (_R * _temperature);
    if ( _initial_He )
    {
      *_initial_He = _n0;
    }
    const Real factor = _t >= _startup_time ? 1.0 : _t / _startup_time;
    _my_value = factor * _initial_pressure;
    if (_output)
    {
      *_output = _my_value;
    }
  }
}
