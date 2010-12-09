#include "PlenumPressureBC.h"

#include "MooseSystem.h"

template<>
InputParameters validParams<PlenumPressureBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<int>("component", "The component for the PlenumPressureBC");
  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the plenum.  If not given, a zero initial pressure will be used.");
  params.addParam<std::string>("material_input", "", "The name of the postprocessor value that holds the amount of material injected into the plenum.");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used.");
  params.addRequiredParam<std::string>("temperature", "The name of the average temperature postprocessor value.");
  params.addRequiredParam<std::string>("volume", "The name of the internal volume postprocessor value.");
  return params;
}

PlenumPressureBC::PlenumPressureBC(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
   _initialized(false),
   _n0(0),
   _component(getParam<int>("component")),
   _initial_pressure(getParam<Real>("initial_pressure")),
   _material_input( getPostprocessorValue(getParam<std::string>("material_input")) ),
   _R(getParam<Real>("R")),
   _temperature( getPostprocessorValue(getParam<std::string>("temperature"))),
   _volume( getPostprocessorValue(getParam<std::string>("volume")))
{
  _moose_system.needPostprocessorsForResiduals( true );

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
PlenumPressureBC::computeQpResidual()
{
  const Real pressure = (_n0 + _material_input) * _R * _temperature / _volume;
  return pressure * (_normals[_qp](_component) * _phi[_i][_qp]);
}

void
PlenumPressureBC::setup()
{
  if ( _initialized )
  {
  }
  else
  {
    _initialized = true;
    _n0 = _initial_pressure * _volume / (_R * _temperature);
  }
}
