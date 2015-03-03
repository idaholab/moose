/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Pressure.h"
#include "Function.h"
#include "MooseError.h"

template<>
InputParameters validParams<Pressure>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<int>("component", "The component for the Pressure");
  params.addParam<Real>("factor", 1.0, "The factor to use in computing the pressure");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  params.addParam<PostprocessorName>("postprocessor", "Postprocessor that will supply the pressure value");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Pressure::Pressure(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _component(getParam<int>("component")),
   _factor(getParam<Real>("factor")),
   _function( isParamValid("function") ? &getFunction("function") : NULL ),
   _postprocessor( isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : NULL )
{

  if (_component < 0 || _component > 2)
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
Pressure::computeQpResidual()
{
  Real factor = _factor;

  if (_function)
  {
    factor *= _function->value(_t, _q_point[_qp]);
  }

  if (_postprocessor)
  {
    factor *= *_postprocessor;
  }

  return factor * (_normals[_qp](_component) * _test[_i][_qp]);
}
