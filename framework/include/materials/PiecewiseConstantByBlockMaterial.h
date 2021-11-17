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
 * Defines a material property that is piecewise constant per block, discontinuous at interfaces
 */
template <bool is_ad>
class PiecewiseConstantByBlockMaterialTempl : public FunctorMaterial
{
public:
  PiecewiseConstantByBlockMaterialTempl(const InputParameters & parameters);
  static InputParameters validParams();

private:
  /// Material property functor defined
  FunctorMaterialProperty<GenericReal<is_ad>> & _prop;
};

typedef PiecewiseConstantByBlockMaterialTempl<false> PiecewiseConstantByBlockMaterial;
typedef PiecewiseConstantByBlockMaterialTempl<true> ADPiecewiseConstantByBlockMaterial;
