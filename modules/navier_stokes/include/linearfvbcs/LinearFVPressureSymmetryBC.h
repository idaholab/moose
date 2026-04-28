//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

#include <vector>

/**
 * Class implementing a symmetry boundary condition for linear finite
 * volume pressure variables used in the pressure corrector equation
 * of segregated fluid dynamics solvers.
 */
class LinearFVPressureSymmetryBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVPressureSymmetryBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

protected:
  /// Refresh per-face/state cache used by the constrained pressure BC path.
  void refreshBoundaryConstraintCache() const;
  /// Normal component of the pressure diffusion coefficient on the current boundary face.
  Real computeBoundaryNormalAinv() const;

  /// Whether the cached constrained pressure gradient is authoritative for this BC.
  const bool _use_constrained_pressure_normal_gradient_only;

  /// Optional total pressure-predictor flux. When provided it supersedes the split H/A + extras.
  const Moose::Functor<Real> * _pressure_predictor_flux;

  /// Optional cached normal pressure gradient populated by the Rhie-Chow constraint update.
  const Moose::Functor<Real> * _constrained_pressure_normal_gradient;

  /// The H/A flux functor for this BC (can be variable, function, etc)
  const Moose::Functor<Real> & _HbyA_flux;

  /// The 1/A tensor serving as a diffusion coefficient, needed when a constrained gradient is used.
  const Moose::Functor<RealVectorValue> * _Ainv;

  /// Additional pressure-equation source-flux functors that must be canceled at the boundary.
  std::vector<const Moose::Functor<Real> *> _additional_face_fluxes;

  /// Cached boundary face info for the currently evaluated face/state.
  mutable const FaceInfo * _cached_face_info = nullptr;
  mutable FaceInfo::VarFaceNeighbors _cached_face_type = FaceInfo::VarFaceNeighbors::BOTH;
  mutable unsigned int _cached_state = 0;
  mutable Moose::SolutionIterationType _cached_iteration_type =
      Moose::SolutionIterationType::Time;
  mutable bool _boundary_constraint_cache_valid = false;
  mutable Real _cached_boundary_pressure_source_flux = 0.0;
  mutable Real _cached_boundary_normal_ainv = 0.0;
  mutable Real _cached_constrained_pressure_normal_gradient = 0.0;
};
