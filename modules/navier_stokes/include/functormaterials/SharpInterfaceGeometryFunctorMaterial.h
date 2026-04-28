#pragma once

#include "FunctorMaterial.h"

#include <array>

/**
 * Functor material that prepares face-aware sharp-interface geometry quantities
 * for reduced-pressure linear-FV coupling.
 *
 * This object is intentionally focused on the *consumer-facing* quantities that
 * the custom Rhie-Chow object needs right now:
 *
 * - clipped / near-interface alpha indicators
 * - face unit normals from grad(alpha)
 * - sigma*kappa (when a curvature functor is supplied)
 * - surface-tension face acceleration
 * - surface-tension cell acceleration
 * - surface-tension momentum-source density components
 * - hydrostatic density-gradient face acceleration
 * - hydrostatic density-gradient cell acceleration
 * - hydrostatic momentum-source density components
 *
 * The full OpenFOAM-like curvature reconstruction path still needs a dedicated
 * curvature producer. For now this material can consume an externally supplied
 * curvature functor and combine it consistently with face gradients and density.
 */
class SharpInterfaceGeometryFunctorMaterial final : public FunctorMaterial
{
public:
  static InputParameters validParams();

  SharpInterfaceGeometryFunctorMaterial(const InputParameters & parameters);

private:
  const Moose::Functor<Real> & _volume_fraction;
  const Moose::Functor<Real> & _density;
  const Moose::Functor<Real> & _surface_tension;
  const Moose::Functor<Real> * const _curvature;
  const Moose::Functor<RealVectorValue> * const _face_smoothed_alpha_gradient;
  const Moose::Functor<RealVectorValue> * const _precomputed_interface_unit_normal;

  const Point _reference_pressure_point;
  const RealVectorValue _gravity;
  const bool _clip_volume_fraction;
  const Real _alpha_lower_bound;
  const Real _alpha_upper_bound;
  const Real _near_interface_lower;
  const Real _near_interface_upper;
  const Real _minimum_density;
  const Real _delta_n;

  const MooseFunctorName _delta_n_name;
  const MooseFunctorName _near_interface_name;
  const MooseFunctorName _alpha_gradient_name;
  const MooseFunctorName _face_smoothed_alpha_gradient_name;
  const MooseFunctorName _density_gradient_name;
  const MooseFunctorName _interface_unit_normal_name;
  const MooseFunctorName _sigma_k_name;
  const MooseFunctorName _reduced_pressure_head_name;
  const MooseFunctorName _surface_tension_face_acceleration_name;
  const MooseFunctorName _surface_tension_cell_acceleration_name;
  const std::array<MooseFunctorName, 3> _surface_tension_momentum_source_names;
  const MooseFunctorName _hydrostatic_density_gradient_face_acceleration_name;
  const MooseFunctorName _hydrostatic_density_gradient_cell_acceleration_name;
  const std::array<MooseFunctorName, 3> _hydrostatic_momentum_source_names;
};
