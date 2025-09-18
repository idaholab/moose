//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SlopeReconstructionMultiD.h"

/**
 * Multi-D least-squares linear reconstruction for SWE [h, hu, hv].
 *
 * - Computes per-cell gradients using face-neighbor cell averages.
 * - Optionally guards positivity of reconstructed h at face centroids.
 * - Caches per-side geometry (centroids/areas) for limiters.
 */
class SlopeReconstructionMultiDSWE : public SlopeReconstructionMultiD
{
public:
  static InputParameters validParams();

  SlopeReconstructionMultiDSWE(const InputParameters & parameters);

  virtual void reconstructElementSlope() override;

protected:
  // Coupled conserved variables
  MooseVariable * _h_var;
  MooseVariable * _hu_var;
  MooseVariable * _hv_var;

  // Options
  const unsigned int _min_neighbors; // minimum neighbors for LSQ; else zero slope
  const MooseEnum _weight_model;     // none | inverse_distance2
  const Real _tikhonov_eps;          // small diagonal regularization
  const Real _dry_depth;             // depth threshold for dry state
  const bool _positivity_guard;      // enforce h(face) >= dry_depth
  const Real _positivity_eps;        // small epsilon for positivity guard

  // helpers
  Real weight(const Point & dx) const;
};

