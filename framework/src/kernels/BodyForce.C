/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BodyForce.h"

#include "Function.h"

template<>
InputParameters validParams<BodyForce>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("value", "The body force value (or scale factor if used with a function)");
  params.addParam<std::string>("function", "", "A function that describes the body force");
  return params;
}

BodyForce::BodyForce(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _value(getParam<Real>("value")),
   _has_function(getParam<std::string>("function") != ""),
   _function( _has_function ? &getFunction("function") : NULL )
{}

Real
BodyForce::computeQpResidual()
{
  Real factor = _value;
  if (_has_function)
  {
    factor *= _function->value(_t,
                               _q_point[_qp](0),
                               _q_point[_qp](1),
                               _q_point[_qp](2));
  }
  return -_test[_i][_qp]*factor;
}

