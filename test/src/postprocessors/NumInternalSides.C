//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumInternalSides.h"

registerMooseObject("MooseTestApp", NumInternalSides);

InputParameters
NumInternalSides::validParams()
{
  InputParameters params = InternalSidePostprocessor::validParams();
  return params;
}

NumInternalSides::NumInternalSides(const InputParameters & parameters)
  : InternalSidePostprocessor(parameters), _count(0)
{
}

NumInternalSides::~NumInternalSides() {}

void
NumInternalSides::execute()
{
  _count++;
}

void
NumInternalSides::initialize()
{
  _count = 0;
}

void
NumInternalSides::finalize()
{
  gatherSum(_count);
}

PostprocessorValue
NumInternalSides::getValue()
{
  return _count;
}

void
NumInternalSides::threadJoin(const UserObject & uo)
{
  const NumInternalSides & obj = static_cast<const NumInternalSides &>(uo);
  _count += obj.count();
}
