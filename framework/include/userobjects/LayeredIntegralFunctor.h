//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredIntegralBase.h"
#include "ElementIntegralFunctorUserObject.h"

/**
 * Computes layered element integrals of a functor.
 */
class LayeredIntegralFunctor : public LayeredIntegralBase<ElementIntegralFunctorUserObject>
{
public:
  static InputParameters validParams();

  LayeredIntegralFunctor(const InputParameters & parameters);
};
