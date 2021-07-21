//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "FunctorInterface.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

class FunctorPropertyValue
{
public:
  FunctorPropertyValue() = default;

  virtual ~FunctorPropertyValue() = default;
};

template <typename T>
class FunctorMaterialProperty : public FunctorPropertyValue, public GenericFunctor<T>
{
public:
  FunctorMaterialProperty(const std::string & name)
    : FunctorPropertyValue(), GenericFunctor<T>(name)
  {
  }

  using typename GenericFunctor<T>::FaceArg;
  using typename GenericFunctor<T>::FunctorType;
  using typename GenericFunctor<T>::FunctorReturnType;
};
