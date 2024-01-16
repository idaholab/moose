//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Computes a 'mass matrix', which will just be a diagonal matrix for the finite volume method,
 * meant for use in preconditioning schemes which require one
 */
class FVMassMatrix : public FVElementalKernel
{
public:
  static InputParameters validParams();

  FVMassMatrix(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  ADReal computeQpResidual() override;

  /// Elemental weighting functor
  const Moose::Functor<Real> & _density;
};
