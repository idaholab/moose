//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorVariableMagnitudeAux.h"

registerMooseObject("MooseApp", VectorVariableMagnitudeAux);

InputParameters
VectorVariableMagnitudeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Creates a field consisting of the magnitude of a "
                             "coupled vector variable.");
  params.addRequiredCoupledVar("vector_variable",
                               "The variable from which to compute the component");
  return params;
}

VectorVariableMagnitudeAux::VectorVariableMagnitudeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _variable_value(coupledVectorValue("vector_variable"))
{
}

Real
VectorVariableMagnitudeAux::computeValue()
{
  return std::sqrt(Utility::pow<2>(_variable_value[_qp](0)) +
                   Utility::pow<2>(_variable_value[_qp](1)) +
                   Utility::pow<2>(_variable_value[_qp](2)));
}
