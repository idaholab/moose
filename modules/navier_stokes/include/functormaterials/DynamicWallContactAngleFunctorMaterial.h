//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "MooseTypes.h"

#include <array>
#include <unordered_map>
#include <vector>

/**
 * Face-aware dynamic wall-contact-angle producer for sharp-interface FV work.
 *
 * This material evaluates a dynamic contact-angle theta(U, nHat) law for sharp-interface
 * transport.
 * It consumes the provisional face unit normal produced by the curvature
 * calculator *before* wall contact-angle correction, so the dynamic model sees
 * the same face-smoothed alpha-gradient / deltaN regularization path used by
 * the curvature reconstruction.
 */
class DynamicWallContactAngleFunctorMaterial final : public FunctorMaterial
{
public:
  static InputParameters validParams();

  DynamicWallContactAngleFunctorMaterial(const InputParameters & parameters);

private:
  struct DynamicBoundarySpec
  {
    Real theta0_deg = 0.0;
    Real theta_adv_deg = 0.0;
    Real theta_rec_deg = 0.0;
    Real u_theta = 0.0;
  };

  void initializeDynamicBoundarySpecs();
  bool getDynamicBoundarySpec(const FaceInfo * fi,
                              BoundaryID & boundary_id,
                              const DynamicBoundarySpec *& spec) const;
  Real evaluateWallContactAngleDegrees(const Moose::FaceArg & face_arg,
                                       const Moose::StateArg & time_arg) const;
  template <typename Arg>
  Real evaluateWallContactAngleDegrees(const Arg &, const Moose::StateArg &) const
  {
    return std::numeric_limits<Real>::quiet_NaN();
  }

  RealVectorValue evaluateBoundaryInternalVelocity(const Moose::FaceArg & face_arg,
                                                   const Moose::StateArg & time_arg) const;
  RealVectorValue evaluateBoundaryWallVelocity(const Moose::FaceArg & face_arg,
                                               const Moose::StateArg & time_arg) const;

  const Moose::Functor<RealVectorValue> & _provisional_interface_unit_normal;
  std::array<const Moose::Functor<Real> *, 3> _velocity_component_functors;
  const Moose::Functor<RealVectorValue> * const _wall_velocity_functor;

  const RealVectorValue _default_wall_velocity;
  const Real _parallel_direction_small;
  const Real _u_theta_small;
  const MooseFunctorName _wall_contact_angle_degrees_name;

  std::unordered_map<BoundaryID, DynamicBoundarySpec> _dynamic_boundary_specs;
};
