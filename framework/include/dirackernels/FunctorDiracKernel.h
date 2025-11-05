//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDiracKernel.h"

/**
 * Computes a dirac source using a functor.
 */
class FunctorDiracKernel : public ADDiracKernel
{
public:
  static InputParameters validParams();

  FunctorDiracKernel(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual ADReal computeQpResidual() override;

  /// Source functor
  const Moose::Functor<ADReal> & _functor;
  /// Source point
  const Point & _p;
};
