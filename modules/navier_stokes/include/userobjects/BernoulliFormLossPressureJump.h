//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "PressureJumpModel.h"

#include <unordered_map>

/**
 * Computes reversible Bernoulli and irreversible form-loss pressure jumps.
 */
class BernoulliFormLossPressureJump : public PressureJumpModel
{
public:
  /// Return the input parameters for this pressure-jump model.
  static InputParameters validParams();

  /**
   * Construct the Bernoulli and form-loss pressure-jump model.
   * @param params The input parameters for the model
   */
  BernoulliFormLossPressureJump(const InputParameters & params);

  /**
   * Compute pressure on the non-owner side minus pressure on the owner side.
   * @param fi The internal face carrying the pressure jump
   * @param face_mass_flux The signed mass flux through the face
   */
  Real computePressureJump(const FaceInfo & fi, Real face_mass_flux) const override;

  /**
   * Return whether reconstruction should use independent values from each side of the face.
   * @param fi The internal face carrying the pressure jump
   */
  bool useOneSidedReconstruction(const FaceInfo & fi) const override;

private:
  /// Porosity on each side of a pressure-jump face.
  const Moose::Functor<Real> & _porosity;

  /// Density on each side of a pressure-jump face.
  const Moose::Functor<Real> & _density;

  /// Whether the reversible Bernoulli term uses one interpolated face density.
  const bool _use_interpolated_density;

  /// Form-loss coefficient associated with each pressure-jump boundary.
  std::unordered_map<BoundaryID, Real> _form_loss_by_id;

  /// Whether each boundary uses the higher-porosity side for its form-loss reference velocity.
  std::unordered_map<BoundaryID, bool> _use_higher_porosity_by_id;
};
