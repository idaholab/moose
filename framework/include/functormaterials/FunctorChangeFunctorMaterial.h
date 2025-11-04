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

/**
 * Adds a functor material property that computes the change in a functor value over a time
 * step, fixed point iteration, or nonlinear iteration.
 */
template <bool is_ad>
class FunctorChangeFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FunctorChangeFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Returns the state argument to use for the reference value
  Moose::StateArg referenceState(const MooseEnum & change_over) const;

  /// Functor for which to evaluate change
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Reference value state argument
  const Moose::StateArg _ref_state;

  /// If true, take the absolute value of the change
  const bool _take_absolute_value;

  /// Name to give functor material property
  const std::string & _prop_name;
};

typedef FunctorChangeFunctorMaterialTempl<false> FunctorChangeFunctorMaterial;
typedef FunctorChangeFunctorMaterialTempl<true> ADFunctorChangeFunctorMaterial;
