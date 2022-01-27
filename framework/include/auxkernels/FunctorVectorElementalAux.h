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
 * Evaluate a functor vector (material property usually) with the element as the functor argument
 * and save one component to an auxiliary variable
 */
template <bool is_ad>
class FunctorVectorElementalAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  FunctorVectorElementalAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Reference to the material property
  const Moose::Functor<GenericRealVectorValue<is_ad>> & _functor;

  /// The component to retrieve
  const unsigned int _component;

  /// Factor to multiply the functor with
  const Moose::Functor<GenericReal<is_ad>> & _factor;
};

typedef FunctorVectorElementalAuxTempl<false> FunctorVectorElementalAux;
typedef FunctorVectorElementalAuxTempl<true> ADFunctorVectorElementalAux;
