#include "Pressure.h"

#include "Function.h"
#include "Moose.h"

template<>
InputParameters validParams<Pressure>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<int>("component", "The component for the Pressure");
  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<std::string>("function", "", "The function that describes the pressure");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Pressure::Pressure(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _component(getParam<int>("component")),
   _factor(getParam<Real>("factor")),
   _function( getParam<std::string>("function") != "" ? &getFunction("function") : NULL )
{
  if(_component < 0 || _component > 2)
  {
    std::stringstream errMsg;
    errMsg << "Invalid component given for "
           << name
           << ": "
           << _component
           << "." << std::endl;

    mooseError( errMsg.str() );
  }

  mooseAssert( getParam<std::string>("function") == "" || _function, "Function not found" );
}

Real
Pressure::computeQpResidual()
{
  Real factor = _factor;

  if (_function)
  {
    factor *= _function->value(_t, _q_point[_qp]);
  }

  return factor * (_normals[_qp](_component) * _test[_i][_qp]);
}
