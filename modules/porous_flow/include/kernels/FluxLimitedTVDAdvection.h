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
#include "AdvectiveFluxCalculatorBase.h"

// Forward Declaration

/**
 * Advection of the variable with velocity set in the AdvectiveFluxCalculator
 *
 * This implements the flux-limited TVD scheme detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD schemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 *
 * Use the quantities built and cached by AdvectiveFluxCalculator
 * to build the residual and Jacobian contributions corresponding
 * to Kuzmin and Turek's stabilized advection
 */
class FluxLimitedTVDAdvection : public Kernel
{
public:
  static InputParameters validParams();

  FluxLimitedTVDAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  /// The user object that computes Kuzmin and Turek's K_ij, R+ and R-, etc quantities
  const AdvectiveFluxCalculatorBase & _fluo;
};
