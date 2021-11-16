//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptionalMaterialPropertyProxyForward.h"
#include "Material.h"

template <typename T>
void
OptionalMaterialPropertyProxy<T>::resolve()
{
  if (_material->hasMaterialProperty<T>(_name))
    _value = &_material->getMaterialProperty<T>(_name);
}

template <typename T>
void
OptionalADMaterialPropertyProxy<T>::resolve()
{
  if (_material->hasADMaterialProperty<T>(_name))
    _value = &_material->getADMaterialProperty<T>(_name);
}
