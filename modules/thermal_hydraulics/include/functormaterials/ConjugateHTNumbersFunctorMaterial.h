//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class SinglePhaseFluidProperties;

/**
 * Computes several non-dimensional numbers for conjugate heat transfer.
 */
template <bool is_ad>
class ConjugateHTNumbersFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ConjugateHTNumbersFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Fluid pressure
  const Moose::Functor<GenericReal<is_ad>> & _p_fluid;
  /// Fluid temperature
  const Moose::Functor<GenericReal<is_ad>> & _T_fluid;
  /// Solid temperature
  const Moose::Functor<GenericReal<is_ad>> & _T_solid;
  /// Characteristic length
  const Real _length;
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};

typedef ConjugateHTNumbersFunctorMaterialTempl<false> ConjugateHTNumbersFunctorMaterial;
typedef ConjugateHTNumbersFunctorMaterialTempl<true> ADConjugateHTNumbersFunctorMaterial;
