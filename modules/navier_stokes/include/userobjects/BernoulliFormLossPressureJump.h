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
  static InputParameters validParams();
  BernoulliFormLossPressureJump(const InputParameters & params);

  Real computePressureJump(const FaceInfo & fi, Real face_mass_flux) const override;
  bool useOneSidedReconstruction(const FaceInfo & fi) const override;

private:
  const Moose::Functor<Real> & _porosity;
  const Moose::Functor<Real> & _density;
  const bool _use_interpolated_density;
  std::unordered_map<BoundaryID, Real> _form_loss_by_id;
  std::unordered_map<BoundaryID, bool> _use_higher_porosity_by_id;
};
