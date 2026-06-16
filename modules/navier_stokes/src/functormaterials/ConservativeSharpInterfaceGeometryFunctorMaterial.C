//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceGeometryFunctorMaterial.h"

#include "MooseFunctorArguments.h"
#include "SubProblem.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceGeometryFunctorMaterial);

namespace
{
Real
geometryClampValue(const Real value, const Real lower, const Real upper)
{
  return std::max(lower, std::min(value, upper));
}

Real
geometrySafeMagnitude(const RealVectorValue & v)
{
  using std::sqrt;
  return sqrt(v * v);
}

}

InputParameters
ConservativeSharpInterfaceGeometryFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addClassDescription(
      "Create face-aware sharp-interface geometry functors for reduced-pressure linear-FV "
      "multiphase coupling.");

  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction_functor",
      "The phase / volume-fraction functor used to construct interface gradients and normals.");
  params.addParam<bool>(
      "clip_volume_fraction_for_geometry",
      true,
      "Whether to clip the volume fraction before computing near-interface indicators.");
  params.addParam<Real>("alpha_lower_bound", 0.0, "Lower clipping bound for the volume fraction.");
  params.addParam<Real>("alpha_upper_bound", 1.0, "Upper clipping bound for the volume fraction.");
  params.addParam<Real>(
      "near_interface_lower", 0.01, "Lower threshold used for the near-interface indicator.");
  params.addParam<Real>(
      "near_interface_upper", 0.99, "Upper threshold used for the near-interface indicator.");
  params.addParam<Real>(
      "delta_n", 1e-8, "Regularization added to |grad(alpha)| when constructing unit normals.");

  params.addParam<MooseFunctorName>(
      "delta_n_name", "delta_n", "Output name for the delta_n functor.");
  params.addParam<MooseFunctorName>(
      "near_interface_name", "near_interface", "Output name for the near-interface indicator.");
  params.addParam<MooseFunctorName>(
      "alpha_gradient_name",
      "alpha_gradient",
      "Output name for the cell gradient of the volume-fraction functor.");
  params.addParam<MooseFunctorName>("interface_unit_normal_name",
                                    "interface_unit_normal_face",
                                    "Output name for the face-oriented unit normal functor.");
  return params;
}

ConservativeSharpInterfaceGeometryFunctorMaterial::
    ConservativeSharpInterfaceGeometryFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _volume_fraction(getFunctor<Real>("volume_fraction_functor")),
    _clip_volume_fraction(getParam<bool>("clip_volume_fraction_for_geometry")),
    _alpha_lower_bound(getParam<Real>("alpha_lower_bound")),
    _alpha_upper_bound(getParam<Real>("alpha_upper_bound")),
    _near_interface_lower(getParam<Real>("near_interface_lower")),
    _near_interface_upper(getParam<Real>("near_interface_upper")),
    _delta_n(getParam<Real>("delta_n")),
    _delta_n_name(getParam<MooseFunctorName>("delta_n_name")),
    _near_interface_name(getParam<MooseFunctorName>("near_interface_name")),
    _alpha_gradient_name(getParam<MooseFunctorName>("alpha_gradient_name")),
    _interface_unit_normal_name(getParam<MooseFunctorName>("interface_unit_normal_name"))
{
  if (_alpha_lower_bound > _alpha_upper_bound)
    paramError("alpha_upper_bound", "alpha_upper_bound must be >= alpha_lower_bound.");

  if (_near_interface_lower > _near_interface_upper)
    paramError("near_interface_upper", "near_interface_upper must be >= near_interface_lower.");

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
      [this](const auto & r, const auto & t) -> RealVectorValue
      { return _volume_fraction.gradient(r, t); },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      _interface_unit_normal_name,
      [this](const auto & r, const auto & t) -> RealVectorValue
      {
        const auto grad_alpha = _volume_fraction.gradient(r, t);
        const Real mag_grad_alpha = geometrySafeMagnitude(MetaPhysicL::raw_value(grad_alpha));

        return grad_alpha / (mag_grad_alpha + _delta_n);
      },
      clearance_schedule);
}
