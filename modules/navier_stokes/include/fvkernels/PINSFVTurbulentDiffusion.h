//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

/**
 * A flux kernel for diffusing turbulent energy in porous media across cell faces, using a scalar
 * isotropic diffusion coefficient, using functor material properties
 */
class PINSFVTurbulentDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVTurbulentDiffusion(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// turbulent viscosity
  const Moose::Functor<ADReal> & _mu_t;
  /// the porosity
  const Moose::Functor<ADReal> & _eps;
  /// regularization turbulent coeffient - divides diffusion
  const Moose::Functor<ADReal> & _turb_coef;
  /// whether the diffusivity should be multiplied by porosity
  const bool _porosity_factored_in;
};