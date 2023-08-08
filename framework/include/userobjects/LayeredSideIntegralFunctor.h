//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LayeredSideIntegralBase.h"
#include "SideIntegralFunctorUserObject.h"

/**
 * Computes layered side integrals of a functor.
 */
class LayeredSideIntegralFunctor : public LayeredSideIntegralBase<SideIntegralFunctorUserObject>
{
public:
  static InputParameters validParams();

  LayeredSideIntegralFunctor(const InputParameters & parameters);
};
