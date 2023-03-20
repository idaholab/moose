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
 * Second time derivative of a variable
 */
class SecondTimeDerivativeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SecondTimeDerivativeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Variable we're computing the second time derivative of
  const VariableValue & _v;

  /// Factor to multiply the second time derivative with
  const Moose::Functor<Real> & _factor;

  /// Whether to use a quadrature-based functor argument, appropriate for finite element
  /// evaluations. If false, use a cell-center functor argument appropriate for finite volume
  /// calculations. The functor argument is used only for the factor for now
  const bool _use_qp_arg;
};
