//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowMassFlux.h"
#include "FaceCenteredMapFunctor.h"
#include <unordered_set>

class PressureJumpModel;

/**
 * Computes Rhie-Chow face mass fluxes for porous flow with optional internal pressure jumps.
 *
 * This object supplies side-specific porosity values to porous momentum kernels, maintains the
 * relaxed pressure jump on each modeled baffle face, and applies the jump consistently to pressure
 * reconstruction and face fluxes.
 */
class PorousRhieChowMassFlux : public RhieChowMassFlux
{
public:
  static InputParameters validParams();

  /**
   * Construct the porous Rhie-Chow mass flux object.
   * @param params The input parameters for the object
   */
  PorousRhieChowMassFlux(const InputParameters & params);

  /**
   * Return the porosity on one side of a face.
   * @param fi The face on which to evaluate porosity
   * @param elem_side Whether to use the element side instead of the neighbor side
   * @param time The solution state at which to evaluate porosity
   */
  Real getFaceSidePorosity(const FaceInfo & fi,
                           bool elem_side,
                           const Moose::StateArg & time) const override;

  /**
   * Return the pressure jump seen from one side of a baffle face.
   * @param fi The face on which to query the jump
   * @param elem_side Whether to return the jump seen from the element side
   */
  Real getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const override;

  /// Whether pressure and velocity reconstruction use separate one-sided stencils on this face.
  bool faceUsesOneSidedReconstruction(const FaceInfo & fi) const override;

  /// Populate baffle-jump storage before initializing the face mass flux.
  void initFaceMassFlux() override;

  /// Populate baffle-jump storage after initializing the pressure coupling field.
  void initCouplingField() override;

  /// Clear face-indexed baffle-jump data after a mesh change.
  void meshChanged() override;

  /// Reset the current baffle-jump values during user object initialization.
  void initialize() override;

protected:
  /// Cache cell porosity in the pressure-system degree-of-freedom ordering.
  void setupMeshInformation() override;

  /// Recompute and relax the pressure jumps using the current face mass fluxes.
  void updateBaffleJumps() override;

  /// Whether the face is governed by one of the configured pressure-jump models.
  bool isBaffleFace(const FaceInfo & fi) const override;

  /// Multiply a pressure-system vector pointwise by the cached cell porosity.
  void applyCellPorosityScaling(NumericVector<Number> & vec) const override;

private:
  /// Whether the face uses the one-term pressure-gradient reconstruction selected by sideset.
  bool isPressureGradientLimited(const FaceInfo & fi) const;

  /// Return the pressure-jump model that applies to the face, or null if there is none.
  const PressureJumpModel * getPressureJumpModel(const FaceInfo & fi) const;

  /// Porosity functor.
  const Moose::Functor<Real> & _eps;

  /// Sidesets using one-term pressure-gradient reconstruction.
  std::unordered_set<BoundaryID> _pressure_gradient_limiter_ids;

  /// Pressure-jump models with mutually disjoint boundary sets.
  std::vector<const PressureJumpModel *> _pressure_jump_models;

  /// Under-relaxation factor applied when updating pressure jumps.
  const Real _pressure_jump_relaxation;

  /// Cell porosity stored in the pressure-system degree-of-freedom ordering.
  std::unique_ptr<NumericVector<Number>> _cell_porosity;

  /// Restartable face field storing pressure as non-owner minus owner.
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & _baffle_jump;
};
