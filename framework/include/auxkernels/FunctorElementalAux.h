//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Evaluate a functor (functor material property, function or variable) with the element as the functor argument
 */
template <bool is_ad>
class FunctorElementalAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  FunctorElementalAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Functor to evaluate with the element argument
  const Moose::Functor<GenericReal<is_ad>> & _functor;
};

typedef FunctorElementalAuxTempl<false> FunctorElementalAux;
typedef FunctorElementalAuxTempl<true> ADFunctorElementalAux;
