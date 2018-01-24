//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExecutionerAttributeReporter.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<ExecutionerAttributeReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  // Parameter for passing in a pointer the attribute being reported (see
  // Executioner::addAttributeReporter)
  params.addPrivateParam<Real *>("value", NULL);
  return params;
}

ExecutionerAttributeReporter::ExecutionerAttributeReporter(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(getCheckedPointerParam<Real *>(
        "value",
        "Invalid pointer to an attribute, this object should only be created via "
        "Executioner::addAttributeReporter"))
{
}

PostprocessorValue
ExecutionerAttributeReporter::getValue()
{
  return *_value;
}
