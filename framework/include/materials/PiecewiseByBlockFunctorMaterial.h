//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Defines a functor material property by another functor (possibly constant) on each block,
 * discontinuous at interfaces
 */
template <bool is_ad>
class PiecewiseByBlockFunctorMaterialTempl : public FunctorMaterial
{
public:
  PiecewiseByBlockFunctorMaterialTempl(const InputParameters & parameters);
  static InputParameters validParams();
};

typedef PiecewiseByBlockFunctorMaterialTempl<false> PiecewiseByBlockFunctorMaterial;
typedef PiecewiseByBlockFunctorMaterialTempl<true> ADPiecewiseByBlockFunctorMaterial;
