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
 * Coupled auxiliary value
 */
template <bool is_ad>
class CoupledAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  CoupledAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  Real _value;         ///< The value being set for this kernel
  MooseEnum _operator; ///< Operator being applied on this variable and coupled variable

  int _coupled;                                     ///< The number of the coupled variable
  const GenericVariableValue<is_ad> & _coupled_val; ///< Coupled variable
};

typedef CoupledAuxTempl<false> CoupledAux;
typedef CoupledAuxTempl<true> ADCoupledAux;
