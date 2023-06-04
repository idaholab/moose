//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedCircumference.h"

registerMooseObject("ThermalHydraulicsApp", GeneralizedCircumference);

InputParameters
GeneralizedCircumference::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<FunctionName>("area_function", "function to compute the cross section");
  params.addClassDescription("Computes a generalized circumference from a function "
                             "providing the area.");
  return params;
}

GeneralizedCircumference::GeneralizedCircumference(const InputParameters & parameters)
  : Function(parameters), FunctionInterface(this), _area_func(getFunction("area_function"))
{
}

Real
GeneralizedCircumference::value(Real t, const Point & p) const
{
  RealVectorValue gradA = _area_func.gradient(t, p);
  Real dAdx2 = gradA(0) * gradA(0);

  // Here, we assume a flow channel with circular cross section.
  return std::sqrt(4. * libMesh::pi * _area_func.value(t, p) + dAdx2);
}

RealVectorValue
GeneralizedCircumference::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
