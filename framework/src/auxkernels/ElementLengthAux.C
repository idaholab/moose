//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementLengthAux.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ElementLengthAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam(
      "method", MooseEnum("min max"), "The size calculation to perform ('min' or 'max').");
  params.addClassDescription(
      "Compute the element size using Elem::hmin() or Elem::hmax() from libMesh.");
  return params;
}

ElementLengthAux::ElementLengthAux(const InputParameters & parameters)
  : AuxKernel(parameters), _use_min(getParam<MooseEnum>("method") == "min")
{
}

Real
ElementLengthAux::computeValue()
{
  if (_use_min)
    return _current_elem->hmin();
  return _current_elem->hmax();
}
