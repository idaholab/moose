//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointerStoreError.h"

registerMooseObject("MooseTestApp", PointerStoreError);

InputParameters
PointerStoreError::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

PointerStoreError::PointerStoreError(const InputParameters & params)
  : GeneralUserObject(params),
    _pointer_data(declareRestartableData<TypeWithNoStore *>("pointer_data"))
{
  _pointer_data = new TypeWithNoStore;
  _pointer_data->_i = 1;
}

PointerStoreError::~PointerStoreError() { delete _pointer_data; }

void
PointerStoreError::initialSetup()
{
  _pointer_data->_i = 2;
}

void
PointerStoreError::timestepSetup()
{
  _pointer_data->_i += 1;
}

void
PointerStoreError::execute()
{
}
