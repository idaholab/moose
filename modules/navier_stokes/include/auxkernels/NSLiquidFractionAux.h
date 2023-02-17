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
 * Computes liquid fraction based on temperature fields
 */
class NSLiquidFractionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSLiquidFractionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// The temperature
  const Moose::Functor<ADReal> & _T;
  /// The solidus temperature
  const Moose::Functor<ADReal> & _T_solidus;
  /// The liquidus temperture
  const Moose::Functor<ADReal> & _T_liquidus;
};
