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
#include "MooseFunctor.h"

class Function;

/**
 * Function auxiliary value
 */
class FunctorCoordinatesFunctionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  FunctorCoordinatesFunctionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Function being used to compute the value of this kernel
  const Function & _func;
  /// Functor being used to provide the 'x' coordinate
  const Moose::Functor<Real> & _x_functor;
  /// Functor being used to provide the 'y' coordinate
  const Moose::Functor<Real> & _y_functor;
  /// Functor being used to provide the 'z' coordinate
  const Moose::Functor<Real> & _z_functor;
  /// Functor being used to provide the 't' coordinate
  const Moose::Functor<Real> & _t_functor;

  /// A factor to multiply the output value with for convenience
  const Moose::Functor<Real> & _factor;

  /// Whether the target variable is finite element
  const bool _is_fe;
};
