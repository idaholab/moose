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
#include <unordered_map>

/**
 * Rhie-Chow mass flux object specialized for porous flow/baffle cases.
 *
 * Rhie-Chow mass flux object specialized for porous flow/baffle cases.
 */
class PorousRhieChowMassFlux : public RhieChowMassFlux
{
public:
  static InputParameters validParams();
  PorousRhieChowMassFlux(const InputParameters & params);

  std::pair<Real, Real> getAdvectedInterpolationCoeffs(const FaceInfo & fi,
                                                       Moose::FV::InterpMethod method,
                                                       Real face_mass_flux,
                                                       bool apply_porosity_scaling = true) const override;
  Real getFaceSidePorosity(const FaceInfo & fi,
                           bool elem_side,
                           const Moose::StateArg & time) const override;
  Real getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const override;
  Real pressureGradient(const ElemInfo & elem_info, unsigned int component) const override;

  void initFaceMassFlux() override;
  void initCouplingField() override;
  void computeFaceMassFlux() override;
  void computeCellVelocity() override;

  void meshChanged() override;
  void initialize() override;

protected:
  const std::vector<std::unique_ptr<NumericVector<Number>>> &
  selectPressureGradient(const bool updated_pressure) override;
  void storePressureGradientFlux(const FaceInfo & fi, Real p_grad_flux) override;
  void setupMeshInformation() override;
  void updateBaffleJumps() override;
  void computeCorrectedPressureGradient() override;
  bool isBaffleFace(const FaceInfo & fi) const override;
  bool elemIsBaffleOwner(const FaceInfo & fi) const override;
  bool isPressureGradientLimited(const FaceInfo & fi) const override;
  bool isReconstructionZeroFluxFace(const FaceInfo & fi) const;
  void applyCellPorosityScaling(NumericVector<Number> & vec) const override;
  bool useHarmonicAinvInterp() const override { return _use_harmonic_Ainv_interp; }
  bool debugBaffle() const override { return _debug_baffle; }
  bool useFluxVelocityReconstruction() const override { return _use_flux_velocity_reconstruction; }

private:
  void updateGradPrevFromFaceVelocity();

  const Moose::Functor<Real> & _eps;

  std::unordered_set<BoundaryID> _pressure_baffle_boundary_ids;
  std::unordered_set<BoundaryID> _pressure_gradient_limiter_ids;
  std::unordered_set<BoundaryID> _reconstruction_zero_flux_boundary_ids;

  const Real _pressure_baffle_relaxation;
  std::unordered_map<BoundaryID, Real> _pressure_baffle_form_loss_by_id;
  std::unordered_map<BoundaryID, bool> _pressure_baffle_form_loss_use_higher_eps_by_id;
  const bool _debug_baffle;
  const bool _use_flux_velocity_reconstruction;
  const Real _flux_velocity_reconstruction_relaxation;
  const bool _use_corrected_pressure_gradient;
  const bool _use_reconstructed_pressure_gradient;
  const Real _pressure_gradient_limiter_blend;
  const bool _use_harmonic_Ainv_interp;

  std::unique_ptr<NumericVector<Number>> _cell_porosity;
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_corrected;
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_p_reconstructed;
  std::vector<std::vector<std::unique_ptr<NumericVector<Number>>>> _grad_w_prev;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _p_grad_flux;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & _baffle_jump;
};
