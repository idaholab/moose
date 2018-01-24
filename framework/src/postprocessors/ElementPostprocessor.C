//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPostprocessor.h"

template <>
InputParameters
validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<ElementUserObject>();
  params += validParams<Postprocessor>();
  return params;
}

ElementPostprocessor::ElementPostprocessor(const InputParameters & parameters)
  : ElementUserObject(parameters), Postprocessor(parameters)
{
}
