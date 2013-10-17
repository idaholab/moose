#include "PlenumPressure.h"
#include "Conversion.h"

template<>
InputParameters validParams<PlenumPressure>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<int>("component", "The component for the PlenumPressure");
  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::vector<PostprocessorName> >("material_input", "The name of the postprocessor(s) that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<PostprocessorName>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<PostprocessorName>("volume", "The name of the internal volume postprocessor value.");
  params.addParam<Real>("startup_time", 0, "The amount of time during which the pressure will ramp from zero to its true value.");
  params.addParam<bool>("output_initial_moles", false, "The reporting postprocessor to use for the initial moles of gas.");
  params.addParam<std::vector<Real> >("refab_time", "The time at which the plenum pressure must be reinitialized due to fuel rod refabrication.");
  params.addParam<std::vector<Real> >("refab_pressure", "The pressure of fill gas at refabrication.");
  params.addParam<std::vector<Real> >("refab_temperature", "The temperature at refabrication.");
  params.addParam<std::vector<Real> >("refab_volume", "The gas volume at refabrication.");
  params.addParam<std::vector<unsigned> >("refab_type", "The type of refabrication.  0 for instantaneous reset of gas, 1 for reset with constant fraction until next refabrication");

  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

PlenumPressure::PlenumPressure(const std::string & name, InputParameters params)
  :IntegratedBC(name, params),
   _component(getParam<int>("component")),
   _n0(declareReportableValue("initial_moles", 0.0,
                              getParam<bool>("output_initial_moles"))),
   _initial_pressure(getParam<Real>("initial_pressure")),
   _material_input(),
   _R(getParam<Real>("R")),
   _temperature( getPostprocessorValue("temperature")),
   _volume( getPostprocessorValue("volume")),
   _start_time(0),
   _startup_time( getParam<Real>("startup_time")),
   _refab_needed(isParamValid("refab_time") ? getParam<std::vector<Real> >("refab_time").size() : 0),
   _refab_gas_released(declareRestartableData<Real>("refab_gas_released", 0)),
   _refab_time( isParamValid("refab_time") ?
                getParam<std::vector<Real> >("refab_time") :
                std::vector<Real>(1, -std::numeric_limits<Real>::max()) ),
   _refab_pressure( isParamValid("refab_pressure") ?
                    getParam<std::vector<Real> >("refab_pressure") :
                    std::vector<Real>(_refab_time.size(), 0) ),
   _refab_temperature( isParamValid("refab_temperature") ?
                       getParam<std::vector<Real> >("refab_temperature") :
                       std::vector<Real>(_refab_time.size(), 0) ),
   _refab_volume( isParamValid("refab_volume") ?
                  getParam<std::vector<Real> >("refab_volume") :
                  std::vector<Real>(_refab_time.size(), 0) ),
   _refab_type( isParamValid("refab_type") ?
                getParam<std::vector<unsigned> >("refab_type") :
                std::vector<unsigned>(_refab_time.size(), 0) ),
   _refab_counter(declareRestartableData<unsigned int>("refab_counter", 0)),
   _my_value(declareReportableValue("ppress", 0.0))
{

  if (isParamValid("material_input"))
  {
    std::vector<PostprocessorName> ppn = params.get<std::vector<PostprocessorName> >("material_input");
    const unsigned len = ppn.size();
    for (unsigned i(0); i < len; ++i)
    {
      _material_input.push_back( &getPostprocessorValueByName(ppn[i]) );
    }
  }

  if (params.isParamValid("refab_time") &&
      !(params.isParamValid("refab_pressure") &&
        params.isParamValid("refab_temperature") &&
        params.isParamValid("refab_volume")))
  {
    mooseError("PlenumPressure error: refabrication time given but not complete set of refabrication data");
  }
  if (_refab_time.size() != _refab_pressure.size() ||
      _refab_pressure.size() != _refab_temperature.size() ||
      _refab_temperature.size() != _refab_volume.size() ||
      _refab_volume.size() != _refab_type.size())
  {
    mooseError("Refab parameters do not have equal lengths");
  }

  if(_component < 0 || _component > 2 )
  {
    std::stringstream errMsg;
    errMsg << "Invalid component given for "
           << name
           << ": "
           << _component
           << "." << std::endl;

    mooseError( errMsg.str() );
  }
}

Real
PlenumPressure::computeQpResidual()
{
  return _my_value * (_normals[_qp](_component) * _test[_i][_qp]);
}

void
PlenumPressure::initialSetup()
{
  if (0 == _refab_counter)
  {
    _n0 = _initial_pressure * _volume / (_R * _temperature);

    _start_time = _t - _dt;
    const Real factor = _t >= _start_time + _startup_time ? 1.0 : (_t-_start_time) / _startup_time;
    _my_value = factor * _initial_pressure;
  } 
}

void
PlenumPressure::timestepSetup()
{
  if (_refab_counter < _refab_needed && _refab_time[_refab_counter] <= _t)
  {
    _refab_gas_released = 0;
    for (unsigned i(0); i < _material_input.size(); ++i)
    {
      _refab_gas_released += *_material_input[i];
    }

    _n0 = _refab_pressure[_refab_counter] * _refab_volume[_refab_counter] / (_R * _refab_temperature[_refab_counter]);

    const Real factor = _t >= _start_time + _startup_time ? 1.0 : (_t-_start_time) / _startup_time;
    _my_value = factor * _refab_pressure[_refab_counter];
    ++_refab_counter;
  }
}

void
PlenumPressure::residualSetup()
{
  Real pressure(0);
  if (!_refab_needed ||
      _refab_counter == 0 || // refab has not occurred
      _refab_type[_refab_counter-1] == 0 ) // flush gas, not hold
  {
    Real mat(0);
    for (unsigned i(0); i < _material_input.size(); ++i)
    {
      mat += *_material_input[i];
    }
    const Real n = _n0 + (mat - _refab_gas_released);
    pressure = n * _R * _temperature / _volume;
  }
  else
  {
    pressure = _refab_pressure[_refab_counter-1];
  }

  const Real factor = _t >= _start_time + _startup_time ? 1.0 : (_t-_start_time) / _startup_time;
  _my_value = factor * pressure;
}
