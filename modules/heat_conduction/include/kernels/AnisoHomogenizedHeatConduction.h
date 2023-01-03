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
 * Extension of HomogenizedHeatConduction to anisotropic thermal conductivities
 */
class AnisoHomogenizedHeatConduction : public Kernel
{
public:
  static InputParameters validParams();

  AnisoHomogenizedHeatConduction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /// the diffusion/thermal conductivity tensor
  const MaterialProperty<RankTwoTensor> & _diffusion_coefficient;

  /// the component of the homogenization characteristic function that is computed
  const unsigned int _component;
};
