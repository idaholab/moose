//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVMomentumDiffusion.h"

/**
 * A flux kernel for diffusion of momentum in porous media across cell faces
 */
class PINSFVMomentumDiffusion : public INSFVMomentumDiffusion
{
public:
  static InputParameters validParams();
  PINSFVMomentumDiffusion(const InputParameters & params);

protected:
  ADReal computeStrongResidual() override;

  /// the porosity
  const Moose::Functor<ADReal> & _eps;
};
