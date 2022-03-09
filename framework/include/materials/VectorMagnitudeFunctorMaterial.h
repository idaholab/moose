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
 * This class takes up to three functors corresponding to vector components and computes the
 * Euclidean norm from them.
 */
template <bool is_ad>
class VectorMagnitudeFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  VectorMagnitudeFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// The x-component functor
  const Moose::Functor<GenericReal<is_ad>> * const _x;
  /// The y-component functor
  const Moose::Functor<GenericReal<is_ad>> & _y;
  /// The z-component functor
  const Moose::Functor<GenericReal<is_ad>> & _z;

  /// Vector functor
  const Moose::Functor<VectorValue<GenericReal<is_ad>>> * const _vector_functor;
};

typedef VectorMagnitudeFunctorMaterialTempl<false> VectorMagnitudeFunctorMaterial;
typedef VectorMagnitudeFunctorMaterialTempl<true> ADVectorMagnitudeFunctorMaterial;
