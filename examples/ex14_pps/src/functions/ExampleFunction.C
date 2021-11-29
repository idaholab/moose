//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleFunction.h"

registerMooseObject("ExampleApp", ExampleFunction);

InputParameters
ExampleFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<Real>("alpha", 1.0, "The value of alpha");
  return params;
}

ExampleFunction::ExampleFunction(const InputParameters & parameters)
  : Function(parameters), _alpha(getParam<Real>("alpha"))
{
}

Real
ExampleFunction::value(Real /*t*/, const Point & p) const
{
  return _alpha * _alpha * libMesh::pi * libMesh::pi *
         std::sin(_alpha * libMesh::pi * p(0)); // p(0) == x
}
