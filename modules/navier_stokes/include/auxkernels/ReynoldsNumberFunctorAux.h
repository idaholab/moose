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
 * Computes rho*u*L/mu where L is the maximum element dimension
 */
class ReynoldsNumberFunctorAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ReynoldsNumberFunctorAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// The fluid speed
  const Moose::Functor<ADReal> & _speed;
  /// The fluid density
  const Moose::Functor<ADReal> & _rho;
  /// The fluid dynamic viscosity
  const Moose::Functor<ADReal> & _mu;
  /// Whether to use a quadrature-based argument to evaluate the functors
  const bool _use_qp_arg;
};
