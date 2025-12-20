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
#include "FVInterpolationMethodInterface.h"
#include <unordered_map>

/**
 * Kernel that adds contributions from an advection term discretized using the finite volume method
 * to a linear system.
 */
class LinearFVAdvection : public LinearFVFluxKernel, public FVInterpolationMethodInterface
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVAdvection(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

protected:
  /// Constant advecting velocity vector
  const RealVectorValue _velocity;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _advected_interp_method;

  /// Optional finite volume interpolation method object for advected quantities
  const FVInterpolationMethod * _adv_interp_method;

  /// Cached handle used to evaluate the advected interpolation method without virtual dispatch
  FVInterpolationMethod::AdvectedFaceInterpolator _adv_interp_handle;

  struct AdvectedCacheEntry
  {
    FVInterpolationMethod::AdvectedInterpolationResult result;
    Real correction_flux; // high-order minus upwind flux contribution (scaled by area)
  };

  /// Cache per face id to avoid recomputing limiter-based weights twice
  mutable std::unordered_map<dof_id_type, AdvectedCacheEntry> _adv_interp_cache;

  /**
   * Compute and cache advected interpolation weights (and deferred correction flux) for current
   * face.
   */
  const AdvectedCacheEntry & computeAdvectedWeights(const Real face_flux);
};
