//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Kernel providing the heat diffusion kernel for example purposes, with
 * strong form $-\nabla\cdot\left(k\nabla T\right)$, where $k$ is the
 * thermal conductivity and $T$ is the temperature.
 */
class HeatDiffusion : public Kernel
{
public:
  HeatDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  /// thermal conductivity
  const MaterialProperty<Real> & _k;
};
