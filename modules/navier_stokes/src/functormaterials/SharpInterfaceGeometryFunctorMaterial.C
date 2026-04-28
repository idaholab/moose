#include "SharpInterfaceGeometryFunctorMaterial.h"

#include "MooseFunctorArguments.h"
#include "SubProblem.h"

#include <algorithm>
#include <cmath>

registerMooseObject("NavierStokesApp", SharpInterfaceGeometryFunctorMaterial);

namespace
{
Real
geometryClampValue(const Real value, const Real lower, const Real upper)
{
  return std::max(lower, std::min(value, upper));
}

Real
geometrySafeDensity(const Real rho, const Real minimum_density)
{
  return std::max(rho, minimum_density);
}

Real
geometrySafeMagnitude(const RealVectorValue & v)
{
  using std::sqrt;
  return sqrt(v * v);
}
}

InputParameters
SharpInterfaceGeometryFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addClassDescription(
      "Create face-aware sharp-interface geometry functors for reduced-pressure linear-FV "
      "multiphase coupling.");

  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction_functor",
      "The phase / volume-fraction functor used to construct interface gradients and normals.");
  params.addRequiredParam<MooseFunctorName>(
      "density_functor",
      "Mixture density functor. This is used for hydrostatic-density-gradient and capillary "
      "acceleration scaling.");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "0",
      "Surface-tension coefficient functor. A constant numeric string is acceptable.");
  params.addParam<MooseFunctorName>(
      "curvature_functor",
      "",
      "Optional curvature functor. When supplied, sigma*kappa and surface-tension face "
      "acceleration functors are created from it. When left empty, the surface-tension face "
      "acceleration functor evaluates to zero.");
  params.addParam<MooseFunctorName>(
      "face_smoothed_alpha_gradient_functor",
      "",
      "Optional face-smoothed alpha-gradient functor produced by the curvature calculator. "
      "When supplied, it is used in the capillary force path so the same OpenFOAM-like "
      "smoothing is used in both curvature and surface-tension evaluation.");
  params.addParam<MooseFunctorName>(
      "interface_unit_normal_functor",
      "",
      "Optional precomputed face unit-normal functor produced by the curvature calculator. "
      "When supplied, the material forwards that functor instead of rebuilding normals from "
      "the raw volume-fraction gradient.");

  params.addParam<RealVectorValue>(
      "gravity",
      RealVectorValue(),
      "Gravity vector used for reduced-pressure hydrostatic-density-gradient correction.");
  params.addParam<Point>(
      "reference_pressure_point",
      Point(0, 0, 0),
      "Reference point used to compute the reduced-pressure head gh = g.(x-x_ref).");

  params.addParam<bool>(
      "clip_volume_fraction_for_geometry",
      true,
      "Whether to clip the volume fraction before computing near-interface indicators.");
  params.addParam<Real>("alpha_lower_bound", 0.0, "Lower clipping bound for the volume fraction.");
  params.addParam<Real>("alpha_upper_bound", 1.0, "Upper clipping bound for the volume fraction.");
  params.addParam<Real>(
      "near_interface_lower",
      0.01,
      "Lower threshold used for the OpenFOAM-like near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper",
      0.99,
      "Upper threshold used for the OpenFOAM-like near-interface indicator.");
  params.addParam<Real>(
      "minimum_density",
      1e-12,
      "Floor used when dividing by density in acceleration-like face functors.");

  params.addParam<Real>(
      "delta_n",
      1e-8,
      "Regularization added to |grad(alpha)| when constructing unit normals. This is the "
      "direct analog of OpenFOAM's deltaN regularization term.");

  params.addParam<MooseFunctorName>("delta_n_name", "delta_n", "Output name for the delta_n functor.");
  params.addParam<MooseFunctorName>(
      "near_interface_name", "near_interface", "Output name for the near-interface indicator.");
  params.addParam<MooseFunctorName>(
      "alpha_gradient_name", "alpha_gradient", "Output name for the cell gradient of the volume-fraction functor.");
  params.addParam<MooseFunctorName>(
      "face_smoothed_alpha_gradient_name",
      "face_smoothed_alpha_gradient",
      "Output name for the OpenFOAM-like face-smoothed alpha-gradient functor.");
  params.addParam<MooseFunctorName>(
      "density_gradient_name", "density_gradient", "Output name for the density gradient functor.");
  params.addParam<MooseFunctorName>(
      "interface_unit_normal_name",
      "interface_unit_normal_face",
      "Output name for the face-oriented unit normal functor.");
  params.addParam<MooseFunctorName>(
      "sigma_k_name",
      "sigma_k",
      "Output name for the sigma*kappa functor.");
  params.addParam<MooseFunctorName>(
      "reduced_pressure_head_name",
      "reduced_pressure_head",
      "Output name for the reduced-pressure head gh functor.");
  params.addParam<MooseFunctorName>(
      "surface_tension_face_acceleration_name",
      "surface_tension_face_acceleration",
      "Output name for the surface-tension face acceleration functor.");
  params.addParam<MooseFunctorName>(
      "surface_tension_cell_acceleration_name",
      "surface_tension_cell_acceleration",
      "Output name for the surface-tension cell acceleration functor.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_x_name",
      "surface_tension_momentum_source_x",
      "Output name for the x-component surface-tension momentum source density.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_y_name",
      "surface_tension_momentum_source_y",
      "Output name for the y-component surface-tension momentum source density.");
  params.addParam<MooseFunctorName>(
      "surface_tension_momentum_source_z_name",
      "surface_tension_momentum_source_z",
      "Output name for the z-component surface-tension momentum source density.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_face_acceleration_name",
      "hydrostatic_density_gradient_face_acceleration",
      "Output name for the hydrostatic-density-gradient face acceleration functor.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_cell_acceleration_name",
      "hydrostatic_density_gradient_cell_acceleration",
      "Output name for the hydrostatic-density-gradient cell acceleration functor.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_x_name",
      "hydrostatic_momentum_source_x",
      "Output name for the x-component reduced-pressure hydrostatic momentum source density.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_y_name",
      "hydrostatic_momentum_source_y",
      "Output name for the y-component reduced-pressure hydrostatic momentum source density.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_momentum_source_z_name",
      "hydrostatic_momentum_source_z",
      "Output name for the z-component reduced-pressure hydrostatic momentum source density.");

  return params;
}

SharpInterfaceGeometryFunctorMaterial::SharpInterfaceGeometryFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _volume_fraction(getFunctor<Real>("volume_fraction_functor")),
    _density(getFunctor<Real>("density_functor")),
    _surface_tension(getFunctor<Real>("surface_tension_coefficient")),
    _curvature(getParam<MooseFunctorName>("curvature_functor").empty()
                   ? nullptr
                   : &getFunctor<Real>("curvature_functor")),
    _face_smoothed_alpha_gradient(
        getParam<MooseFunctorName>("face_smoothed_alpha_gradient_functor").empty()
            ? nullptr
            : &getFunctor<RealVectorValue>("face_smoothed_alpha_gradient_functor")),
    _precomputed_interface_unit_normal(
        getParam<MooseFunctorName>("interface_unit_normal_functor").empty()
            ? nullptr
            : &getFunctor<RealVectorValue>("interface_unit_normal_functor")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _clip_volume_fraction(getParam<bool>("clip_volume_fraction_for_geometry")),
    _alpha_lower_bound(getParam<Real>("alpha_lower_bound")),
    _alpha_upper_bound(getParam<Real>("alpha_upper_bound")),
    _near_interface_lower(getParam<Real>("near_interface_lower")),
    _near_interface_upper(getParam<Real>("near_interface_upper")),
    _minimum_density(getParam<Real>("minimum_density")),
    _delta_n(getParam<Real>("delta_n")),
    _delta_n_name(getParam<MooseFunctorName>("delta_n_name")),
    _near_interface_name(getParam<MooseFunctorName>("near_interface_name")),
    _alpha_gradient_name(getParam<MooseFunctorName>("alpha_gradient_name")),
    _face_smoothed_alpha_gradient_name(
        getParam<MooseFunctorName>("face_smoothed_alpha_gradient_name")),
    _density_gradient_name(getParam<MooseFunctorName>("density_gradient_name")),
    _interface_unit_normal_name(getParam<MooseFunctorName>("interface_unit_normal_name")),
    _sigma_k_name(getParam<MooseFunctorName>("sigma_k_name")),
    _reduced_pressure_head_name(getParam<MooseFunctorName>("reduced_pressure_head_name")),
    _surface_tension_face_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_face_acceleration_name")),
    _surface_tension_cell_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_cell_acceleration_name")),
    _surface_tension_momentum_source_names{
        getParam<MooseFunctorName>("surface_tension_momentum_source_x_name"),
        getParam<MooseFunctorName>("surface_tension_momentum_source_y_name"),
        getParam<MooseFunctorName>("surface_tension_momentum_source_z_name")},
    _hydrostatic_density_gradient_face_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration_name")),
    _hydrostatic_density_gradient_cell_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration_name")),
    _hydrostatic_momentum_source_names{
        getParam<MooseFunctorName>("hydrostatic_momentum_source_x_name"),
        getParam<MooseFunctorName>("hydrostatic_momentum_source_y_name"),
        getParam<MooseFunctorName>("hydrostatic_momentum_source_z_name")}
{
  if (_alpha_lower_bound > _alpha_upper_bound)
    paramError("alpha_upper_bound", "alpha_upper_bound must be >= alpha_lower_bound.");

  if (_near_interface_lower > _near_interface_upper)
    paramError("near_interface_upper", "near_interface_upper must be >= near_interface_lower.");

  if (_minimum_density <= 0)
    paramError("minimum_density", "minimum_density must be positive.");

  if (_delta_n <= 0)
    paramError("delta_n", "delta_n must be positive.");

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  addFunctorProperty<Real>(
      _delta_n_name,
      [this](const auto &, const auto &) -> Real { return _delta_n; },
      clearance_schedule);

  addFunctorProperty<Real>(
      _near_interface_name,
      [this](const auto & r, const auto & t) -> Real
      {
        Real alpha = _volume_fraction(r, t);
        if (_clip_volume_fraction)
          alpha = geometryClampValue(alpha, _alpha_lower_bound, _alpha_upper_bound);
        return (alpha >= _near_interface_lower && alpha <= _near_interface_upper) ? 1.0 : 0.0;
      },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _alpha_gradient_name,
      [this](const auto & r, const auto & t) -> RealVectorValue { return _volume_fraction.gradient(r, t); },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _face_smoothed_alpha_gradient_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        if (_face_smoothed_alpha_gradient)
          return (*_face_smoothed_alpha_gradient)(r, t);
        return _volume_fraction.gradient(r, t);
      },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _density_gradient_name,
      [this](const auto & r, const auto & t) -> RealVectorValue { return _density.gradient(r, t); },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _interface_unit_normal_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        if (_precomputed_interface_unit_normal)
          return (*_precomputed_interface_unit_normal)(r, t);

        const auto grad_alpha = _face_smoothed_alpha_gradient
                                    ? (*_face_smoothed_alpha_gradient)(r, t)
                                    : _volume_fraction.gradient(r, t);
        const Real mag_grad_alpha = geometrySafeMagnitude(MetaPhysicL::raw_value(grad_alpha));

        // This mirrors the OpenFOAM regularization nHat = grad(alpha) / (|grad(alpha)| + deltaN).
        // When the curvature producer is active, boundary contact-angle correction has already
        // been applied to the supplied face-unit-normal functor before this fallback path is used.
        return grad_alpha / (mag_grad_alpha + _delta_n);
      },
      clearance_schedule);

  addFunctorProperty<Real>(
      _sigma_k_name,
      [this](const auto & r, const auto & t) -> Real
      {
        if (!_curvature)
          return 0.0;
        return _surface_tension(r, t) * (*_curvature)(r, t);
      },
      clearance_schedule);

  addFunctorProperty<Real>(
      _reduced_pressure_head_name,
      [this](const auto & r, const auto &) -> Real
      {
        const Point x = r.getPoint();
        return _gravity * (x - _reference_pressure_point);
      },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _surface_tension_face_acceleration_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        if (!_curvature)
          return RealVectorValue();

        const Real rho = geometrySafeDensity(_density(r, t), _minimum_density);
        const Real sigma_k = _surface_tension(r, t) * (*_curvature)(r, t);
        const auto raw_grad_alpha = _face_smoothed_alpha_gradient
                                        ? (*_face_smoothed_alpha_gradient)(r, t)
                                        : _volume_fraction.gradient(r, t);
        const Real mag_grad_alpha =
            geometrySafeMagnitude(MetaPhysicL::raw_value(raw_grad_alpha));
        RealVectorValue effective_grad_alpha = MetaPhysicL::raw_value(raw_grad_alpha);
        if (_precomputed_interface_unit_normal)
          effective_grad_alpha =
              mag_grad_alpha * MetaPhysicL::raw_value((*_precomputed_interface_unit_normal)(r, t));

        // Force-per-mass form so the Rhie-Chow object can multiply by rho_f and raw A^{-1}
        // exactly once when constructing the pressure-predictor face flux.
        //
        // When a curvature producer supplies a pre-corrected face unit normal, including wall
        // contact-angle correction, we rebuild the face gradient from |grad(alpha_f)| and that
        // corrected direction so the capillary predictor uses the same orientation as curvature.
        return (sigma_k / rho) * effective_grad_alpha;
      },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _surface_tension_cell_acceleration_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        if (!_curvature)
          return RealVectorValue();

        const Real rho = geometrySafeDensity(_density(r, t), _minimum_density);
        const Real sigma_k = _surface_tension(r, t) * (*_curvature)(r, t);
        const RealVectorValue grad_alpha = MetaPhysicL::raw_value(_volume_fraction.gradient(r, t));

        return (sigma_k / rho) * grad_alpha;
      },
      clearance_schedule);

  for (const auto component : make_range(_surface_tension_momentum_source_names.size()))
    addFunctorProperty<Real>(
        _surface_tension_momentum_source_names[component],
        [this, component](const auto & r, const auto & t) -> Real
        {
          if (!_curvature)
            return 0.0;

          const Real sigma_k = _surface_tension(r, t) * (*_curvature)(r, t);
          const RealVectorValue grad_alpha = MetaPhysicL::raw_value(_volume_fraction.gradient(r, t));

          // Force-per-volume form so the reduced-pressure momentum predictor carries the same
          // capillary term that the face-based pressure-correction path uses in phig.
          return sigma_k * grad_alpha(component);
        },
        clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _hydrostatic_density_gradient_face_acceleration_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        const Real rho = geometrySafeDensity(_density(r, t), _minimum_density);
        const Point x = r.getPoint();
        const Real gh = _gravity * (x - _reference_pressure_point);
        const RealVectorValue grad_rho = MetaPhysicL::raw_value(_density.gradient(r, t));

        // This maps directly to the OpenFOAM-style reduced-pressure face predictor term
        // - gh_f * snGrad(rho) after the Rhie-Chow object projects onto the face normal and
        // multiplies by rho_f * A^{-1}_{raw,f}.
        return -(gh / rho) * grad_rho;
      },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _hydrostatic_density_gradient_cell_acceleration_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        const Real rho = geometrySafeDensity(_density(r, t), _minimum_density);
        const Point x = r.getPoint();
        const Real gh = _gravity * (x - _reference_pressure_point);
        const RealVectorValue grad_rho = MetaPhysicL::raw_value(_density.gradient(r, t));

        return -(gh / rho) * grad_rho;
      },
      clearance_schedule);

  for (const auto component : make_range(_hydrostatic_momentum_source_names.size()))
    addFunctorProperty<Real>(
        _hydrostatic_momentum_source_names[component],
        [this, component](const auto & r, const auto & t) -> Real
        {
          const Point x = r.getPoint();
          const Real gh = _gravity * (x - _reference_pressure_point);
          const RealVectorValue grad_rho = MetaPhysicL::raw_value(_density.gradient(r, t));

          return -gh * grad_rho(component);
        },
        clearance_schedule);
}
