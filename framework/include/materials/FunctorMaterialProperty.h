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

/**
 * A non-templated base class for functor material properties that allow an owner object to hold
 * different class template instantiations of \p FunctorMaterialProperty in a single container
 */
class FunctorPropertyValue
{
public:
  FunctorPropertyValue() = default;

  virtual ~FunctorPropertyValue() = default;
};

/**
 * A material property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class FunctorMaterialProperty : public FunctorPropertyValue, public GenericFunctor<T>
{
public:
  FunctorMaterialProperty(const std::string & name, const bool default_property = false)
    : FunctorPropertyValue(), GenericFunctor<T>(name), _default_property(default_property)
  {
  }

  using typename GenericFunctor<T>::FaceArg;
  using typename GenericFunctor<T>::FunctorType;
  using typename GenericFunctor<T>::FunctorReturnType;

protected:
  bool checkMultipleDefinitions() const override final { return !_default_property; }

  /// whether this functor material property is a default material property, e.g. one created when a
  /// user supplied a constant in the input file to a \p MaterialPropertyName parameter
  const bool _default_property;
};
