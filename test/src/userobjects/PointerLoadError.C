//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointerLoadError.h"

registerMooseObject("MooseTestApp", PointerLoadError);

InputParameters
PointerLoadError::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

PointerLoadError::PointerLoadError(const InputParameters & params)
  : GeneralUserObject(params),
    _pointer_data(declareRestartableData<TypeWithNoLoad *>("pointer_data"))
{
  _pointer_data = new TypeWithNoLoad;
  _pointer_data->_i = 1;
}

PointerLoadError::~PointerLoadError() { delete _pointer_data; }

void
PointerLoadError::initialSetup()
{
  _pointer_data->_i = 2;
}

void
PointerLoadError::timestepSetup()
{
  _pointer_data->_i += 1;
}

void
PointerLoadError::execute()
{
}
