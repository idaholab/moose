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
 * Evaluate a functor vector material property with the element as the functor argument
 * and save one component to an auxiliary variable
 */
template <bool is_ad>
class FunctorVectorMatPropElementalAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  FunctorVectorMatPropElementalAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Reference to the material property
  const Moose::Functor<GenericRealVectorValue<is_ad>> & _mat_prop;

  /// The component to retrieve
  const unsigned int _component;
};

typedef FunctorVectorMatPropElementalAuxTempl<false> FunctorVectorMatPropElementalAux;
typedef FunctorVectorMatPropElementalAuxTempl<true> FunctorADVectorMatPropElementalAux;
