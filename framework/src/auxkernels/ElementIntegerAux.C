//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegerAux.h"

registerMooseObject("MooseApp", ElementIntegerAux);

InputParameters
ElementIntegerAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Creates a field showing the element integer.");
  params.addParam<std::vector<ExtraElementIDName>>("integer_names",
                                                   "Element integers to be retrieved");
  return params;
}

ElementIntegerAux::ElementIntegerAux(const InputParameters & parameters)
  : AuxKernel(parameters), _id(getElementID("integer_names"))
{
  if (isNodal())
    mooseError("ElementIntegerAux must be for an elemental variable");
}

Real
ElementIntegerAux::computeValue()
{
  return _id;
}
