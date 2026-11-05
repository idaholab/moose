//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"
#include "RhieChowMassFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "FVAdvectedInterpolationMethod.h"
#include "FVInterpolationMethodInterface.h"

class LinearFVGradientReader;

/**
 * An advection kernel that implements the advection term for the passive scalar transport equation.
 */
class LinearFVScalarAdvection : public LinearFVFluxKernel, public FVInterpolationMethodInterface
{
public:
  static InputParameters validParams();
  LinearFVScalarAdvection(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  /// The Rhie-Chow user object that provides us with the face velocity
  const RhieChowMassFlux & _mass_flux_provider;

private:
  /// The interpolation method to use for the advected quantity
  const FVAdvectedInterpolationMethod & _adv_interp_method;

  /// Gradient field used by advected interpolations that require gradients.
  const LinearFVGradientReader * const _gradient_field;

  /// Scheme the drift flux is interpolated against. The split is unconditional; this only chooses
  /// which scheme the drift half uses, and defaults to the mixture flux's own. Giving the drift a
  /// scheme of its own is what allows it to be limited independently of the mixture flux.
  const FVAdvectedInterpolationMethod & _slip_adv_interp_method;

  /// Gradient field used by the drift interpolation when it requires gradients.
  const LinearFVGradientReader * const _slip_gradient_field;

  /// Cached weights/correction for the current face (refreshed in setupFaceData)
  FVAdvectedInterpolationMethod::AdvectedSystemContribution _adv_interp_result;

  /// Reusable gradient storage used when advected interpolation requires gradients.
  VectorValue<Real> _elem_grad_storage;
  VectorValue<Real> _neighbor_grad_storage;

  /// Cached weights/correction for the drift flux on the current face
  FVAdvectedInterpolationMethod::AdvectedSystemContribution _slip_interp_result;

  /// The same gradient storage for the drift half, kept apart because the two schemes may differ
  VectorValue<Real> _slip_elem_grad_storage;
  VectorValue<Real> _slip_neighbor_grad_storage;

  /// Container for the velocity on the face which will be reused in the advection term's
  /// matrix and right hand side contribution
  Real _volumetric_face_flux;

  /// The drift contribution to the face flux. The total flux carried across the face is always
  /// the sum of this and the mixture flux.
  Real _slip_face_flux;

  /// Optional density multiplying the advective flux, so that the conservative form
  /// div(rho phi u) is assembled rather than div(phi u). Null when absent. The matching 'factor'
  /// must be set on the time derivative kernel for the pair to be consistent.
  const Moose::Functor<Real> * const _density;

  /// slip velocity in direction x
  const Moose::Functor<ADReal> * const _u_slip;
  /// slip velocity in direction y
  const Moose::Functor<ADReal> * const _v_slip;
  /// slip velocity in direction z
  const Moose::Functor<ADReal> * const _w_slip;

  /// Whether to use an additional slip velocity to compute the face flux
  bool _add_slip_model;

  /// Boundaries on which the slip velocity is allowed to contribute to the advective flux. The
  /// dispersed phase cannot cross an impermeable boundary, so the slip is only applied where the
  /// mixture itself can cross, that is on inlets and outlets.
  std::set<BoundaryID> _slip_boundaries;
};
