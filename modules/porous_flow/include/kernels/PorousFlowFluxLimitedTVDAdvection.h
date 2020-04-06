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
#include "PorousFlowAdvectiveFluxCalculatorBase.h"

// Forward Declaration

/**
 * Advection of a quantity with velocity set in the PorousFlowAdvectiveFluxCalculator
 * Depending on the PorousFlowAdvectiveFluxCalculator, the quantity may be
 * either a fluid component in a fluid phase, or heat energy in a fluid phase.
 *
 * This implements the flux-limited TVD scheme detailed in
 * D Kuzmin and S Turek "High-resolution FEM-TVD schemes based on a fully multidimensional flux
 * limiter" Journal of Computational Physics 198 (2004) 131-158
 *
 * This is a simple class: it simply uses the quantities built and cached by
 * PorousFlowAdvectiveFluxCalculator to build the residual and Jacobian
 */
class PorousFlowFluxLimitedTVDAdvection : public Kernel
{
public:
  static InputParameters validParams();

  PorousFlowFluxLimitedTVDAdvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// The user object that computes Kuzmin and Turek's K_ij, R+ and R-, etc quantities
  const PorousFlowAdvectiveFluxCalculatorBase & _fluo;
};
