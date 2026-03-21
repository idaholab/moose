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
class PorousLinearWCNSFVMomentumFlux : public LinearWCNSFVMomentumFlux
{
public:
  static InputParameters validParams();

  PorousLinearWCNSFVMomentumFlux(const InputParameters & params);

  void addMatrixContribution() override;

  void setupFaceData(const FaceInfo * face_info) override;
  Real computeElemMatrixContribution() override;
  Real computeNeighborMatrixContribution() override;
  Real computeElemRightHandSideContribution() override;
  Real computeNeighborRightHandSideContribution() override;

protected:
  Real computeAdvectionBoundaryMatrixContribution(const LinearFVAdvectionDiffusionBC * bc) override;
  Real computeAdvectionBoundaryRHSContribution(const LinearFVAdvectionDiffusionBC * bc) override;

private:
  bool isInternalBaffleFace() const;
  Real computeBaffleAdvectionExplicitCorrection(bool elem_side) const;

  /// Whether to include porosity outside the divergence in the advection term
  const bool _porosity_outside_divergence;
};
