//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVScalarAdvection.h"

/**
 * Advection of the dispersed phase fraction of a two-phase mixture, carried by the mixture flux
 * and by the drift of the dispersed phase relative to the mixture. The drift flux is upwinded and
 * interpolated separately from the mixture flux, and is withheld on impermeable boundaries.
 */
class LinearWCNSFV2PScalarAdvection : public LinearFVScalarAdvection
{
public:
  static InputParameters validParams();
  LinearWCNSFV2PScalarAdvection(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual void setupFaceData(const FaceInfo * face_info) override;

protected:
  virtual Real boundaryFaceFlux() const override { return _volumetric_face_flux + _slip_face_flux; }

private:
  /// Scheme the drift flux is interpolated against. The split is unconditional; this only chooses
  /// which scheme the drift half uses, and defaults to the mixture flux's own. Giving the drift a
  /// scheme of its own is what allows it to be limited independently of the mixture flux.
  const FVAdvectedInterpolationMethod & _slip_adv_interp_method;

  /// Gradient field used by the drift interpolation when it requires gradients.
  const LinearFVGradientReader * const _slip_gradient_field;

  /// Cached weights/correction for the drift flux on the current face
  FVAdvectedInterpolationMethod::AdvectedSystemContribution _slip_interp_result;

  /// The gradient storage for the drift half, kept apart from the mixture half because the two
  /// schemes may differ
  VectorValue<Real> _slip_elem_grad_storage;
  VectorValue<Real> _slip_neighbor_grad_storage;

  /// The drift contribution to the face flux. The total flux carried across the face is always
  /// the sum of this and the mixture flux.
  Real _slip_face_flux;

  /// slip velocity in direction x
  const Moose::Functor<ADReal> & _u_slip;
  /// slip velocity in direction y
  const Moose::Functor<ADReal> * const _v_slip;
  /// slip velocity in direction z
  const Moose::Functor<ADReal> * const _w_slip;

  /// Boundaries on which the slip velocity is allowed to contribute to the advective flux. The
  /// dispersed phase cannot cross an impermeable boundary, so the slip is only applied where the
  /// mixture itself can cross, that is on inlets and outlets.
  std::set<BoundaryID> _slip_boundaries;
};
