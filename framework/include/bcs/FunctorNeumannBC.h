//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Neumann boundary condition with functor inputs.
 */
class FunctorNeumannBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  FunctorNeumannBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The functor to impose
  const Moose::Functor<ADReal> & _functor;
  /// Coefficient
  const Moose::Functor<ADReal> & _coef;
  /// Sign to apply to flux
  const Real _sign;
};
