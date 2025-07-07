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
#include "FVDiffusionInterpolationInterface.h"

/*
 *  FVCoupledValueDiffusionKernel implements a standard diffusion term:
 *
 *      - strong form: \nabla \cdot v \nabla u
 *
 *      - weak form: \int_{A} v \nabla u \cdot \vec{n} dA
 */
class FVCoupledValueDiffusionKernel : public FVFluxKernel, public FVDiffusionInterpolationInterface
{
public:
  static InputParameters validParams();
  FVCoupledValueDiffusionKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Coupled variable value (either FE or FV) in current element
  const ADVariableValue & _v_elem;
  /// Coupled variable value (either FE or FV) in neighboring element
  const ADVariableValue & _v_neighbor;

  /// Decides if a geometric arithmetic or harmonic average is used for the
  /// face interpolation of the diffusion coefficient.
  const Moose::FV::InterpMethod _coeff_interp_method;
};
