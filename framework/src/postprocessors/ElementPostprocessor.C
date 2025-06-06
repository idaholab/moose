//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPostprocessor.h"

InputParameters
ElementPostprocessor::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

ElementPostprocessor::ElementPostprocessor(const InputParameters & parameters)
  : ElementUserObject(parameters), Postprocessor(this)
{
}
