#include "Pressure.h"

#include "Function.h"
#include "Moose.h"

template<>
InputParameters validParams<Pressure>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<int>("component", "The component for the Pressure");
  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<std::string>("function", "", "The function that describes the pressure");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Pressure::Pressure(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
   _component(getParam<int>("component")),
   _factor(getParam<Real>("factor")),
   _has_function(getParam<std::string>("function") != ""),
   _function( _has_function ? &getFunction("function") : NULL )
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

  if (_has_function && !_function)
  {
    std::cout << "Unable to find function in pressure bc." << std::endl;
    libmesh_error();
  }
}

Real
Pressure::computeQpResidual()
{
  Real factor = _factor;

  if (_has_function)
    factor *= _function->value(_t, _q_point[_qp]);

  return factor * (_normals[_qp](_component) * _phi[_i][_qp]);
}
