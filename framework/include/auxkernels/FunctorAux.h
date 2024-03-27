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
 * Evaluate a functor (functor material property, function or variable) with either a cell-center,
 * quadrature point, or node as the functor argument
 */
class FunctorAux : public AuxKernel
{
public:
  static InputParameters validParams();

  FunctorAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Functor to evaluate with the element argument
  const Moose::Functor<Real> & _functor;

  /// Factor to multiply the functor with
  const Moose::Functor<Real> & _factor;

  /// Whether the variable is a standard finite element variable
  const bool _is_standard_fe;

  /// Whether the variable is a standard finite volume variable
  const bool _is_standard_fv;
};
