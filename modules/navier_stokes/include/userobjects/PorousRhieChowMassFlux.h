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
 * Rhie-Chow mass flux object specialized for porous flow/baffle cases.
 */
class PorousRhieChowMassFlux : public RhieChowMassFlux
{
public:
  static InputParameters validParams();
  PorousRhieChowMassFlux(const InputParameters & params);

  Real getFaceSidePorosity(const FaceInfo & fi,
                           bool elem_side,
                           const Moose::StateArg & time) const override;
  Real getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const override;
  bool faceUsesOneSidedReconstruction(const FaceInfo & fi) const override;

  void initFaceMassFlux() override;
  void initCouplingField() override;

  void meshChanged() override;
  void initialize() override;

protected:
  void setupMeshInformation() override;
  void updateBaffleJumps() override;
  bool isBaffleFace(const FaceInfo & fi) const override;
  bool isReconstructionZeroFluxFace(const FaceInfo & fi) const override;
  void applyCellPorosityScaling(NumericVector<Number> & vec) const override;

private:
  bool isPressureGradientLimited(const FaceInfo & fi) const;
  const PressureJumpModel * getPressureJumpModel(const FaceInfo & fi) const;

  const Moose::Functor<Real> & _eps;

  std::unordered_set<BoundaryID> _pressure_gradient_limiter_ids;
  std::unordered_set<BoundaryID> _reconstruction_zero_flux_boundary_ids;

  std::vector<const PressureJumpModel *> _pressure_jump_models;
  const Real _pressure_jump_relaxation;
  std::unique_ptr<NumericVector<Number>> _cell_porosity;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & _baffle_jump;
};
