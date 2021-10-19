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
 * Evaluate a functor material property with the element as the functor argument
 */
template <bool is_ad>
class FunctorMatPropElementalAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  FunctorMatPropElementalAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const Moose::Functor<GenericReal<is_ad>> & _mat_prop;
};

typedef FunctorMatPropElementalAuxTempl<false> FunctorMatPropElementalAux;
typedef FunctorMatPropElementalAuxTempl<true> FunctorADMatPropElementalAux;
