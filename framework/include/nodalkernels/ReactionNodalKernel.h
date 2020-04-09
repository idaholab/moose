//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

/**
 * Represents a nodal reaction term equivalent to $a * u$
 */
class ReactionNodalKernel : public NodalKernel
{
public:
  static InputParameters validParams();

  ReactionNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// An optional input-file supplied rate coefficient
  const Real _coeff;
};
