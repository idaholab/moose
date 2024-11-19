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

/// FVCoupledValueDotDiffusion implements a standard diffusion term:
///
///     - strong form: \nabla \cdot v \nabla u
///
///     - weak form: \int_{A} v \nabla u \cdot \vec{n} dA
///
/// It uses/requests a material property named "coeff" for k. An average of
/// the elem and neighbor k-values (which should be face-values) is used to
/// compute k on the face. Cross-diffusion correction factors are currently not
/// implemented for the "grad_u*n" term.
class FVCoupledValueDotDiffusion : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVCoupledValueDotDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _v_elem;
  const ADVariableValue & _v_neighbor;
};
