//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBaseTempl.h"

/**
 * Dirichlet boundary condition with functor inputs.
 */
class FunctorDirichletBC : public ADDirichletBCBaseTempl<Real>
{
public:
  static InputParameters validParams();

  FunctorDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpValue() override;

  /// The functor value to impose
  const Moose::Functor<ADReal> & _functor;
  /// Coefficient
  const Moose::Functor<ADReal> & _coef;
};
