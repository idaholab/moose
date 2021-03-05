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
 * A flux kernel diffusion of momentum in porous media across cell faces
 */
class PINSFVMomentumPorosityDiscontinuity : public FVFluxKernel
{
public:
  static InputParameters validParams();
  PINSFVMomentumPorosityDiscontinuity(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the local element porosity
  const VariableValue & _eps;
  /// the neighbor element porosity
  const VariableValue & _eps_neighbor;
  /// which momentum component this kernel applies to
  const int _index;
  /// the form loss coefficient
  const Real _K;
};
