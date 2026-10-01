//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearWCNSFVMomentumFlux.h"

/**
 * Momentum flux kernel with porous-specific advection handling.
 */
class LinearPWCNSFVMomentumFlux : public LinearWCNSFVMomentumFlux
{
public:
  static InputParameters validParams();

  LinearPWCNSFVMomentumFlux(const InputParameters & params);

  void addMatrixContribution() override;

  Real computeElemMatrixContribution() override;
  Real computeNeighborMatrixContribution() override;
  Real computeElemRightHandSideContribution() override;
  Real computeNeighborRightHandSideContribution() override;

protected:
  Real computeInternalStressTransmissibility() const override;
  Real computeInternalStressExplicitCorrection() const override;

  Real computeAdvectionBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc) override;
  Real computeAdvectionBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc) override;

private:
  /// Whether to use a two-point harmonic transmissibility for the stress term.
  const bool _use_two_point_stress_transmissibility;

  bool isInternalBaffleFace() const;
  bool needsInternalBaffleAdvectionCorrection() const;
  Real computeBaffleAdvectionExplicitCorrection(bool elem_side) const;
  Real inversePorosity(bool elem_side) const;
};
