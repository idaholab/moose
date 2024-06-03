//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAnisotropicDiffusion.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("MooseApp", LinearFVAnisotropicDiffusion);

InputParameters
LinearFVAnisotropicDiffusion::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of a "
                             "diffusion term in a partial differential equation.");
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "If the nonorthogonal correction should be used when computing the normal gradient.");
  params.addParam<bool>(
      "use_nonorthogonal_correction_on_boundary",
      "If the nonorthogonal correction should be used when computing the normal gradient.");
  params.addRequiredParam<MooseFunctorName>("diffusion_tensor",
                                            "Functor describing a diagonal diffusion tensor.");
  return params;
}

LinearFVAnisotropicDiffusion::LinearFVAnisotropicDiffusion(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _diffusion_tensor(getFunctor<RealVectorValue>("diffusion_tensor")),
    _use_nonorthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _use_nonorthogonal_correction_on_boundary(
        isParamValid("use_nonorthogonal_correction_on_boundary")
            ? getParam<bool>("use_nonorthogonal_correction_on_boundary")
            : _use_nonorthogonal_correction),
    _flux_matrix_contribution(0.0),
    _flux_rhs_contribution(0.0)
{
  _var.computeCellGradients();
}

void
LinearFVAnisotropicDiffusion::initialSetup()
{
  for (const auto bc : _var.getBoundaryConditionMap())
    if (!dynamic_cast<const LinearFVAdvectionDiffusionBC *>(bc.second))
      mooseError(
          bc.second->type(), " is not a compatible boundary condition with ", this->type(), "!");
}

Real
LinearFVAnisotropicDiffusion::computeElemMatrixContribution()
{
  return computeFluxMatrixContribution();
}

Real
LinearFVAnisotropicDiffusion::computeNeighborMatrixContribution()
{
  return -computeFluxMatrixContribution();
}

Real
LinearFVAnisotropicDiffusion::computeElemRightHandSideContribution()
{
  return computeFluxRHSContribution();
}

Real
LinearFVAnisotropicDiffusion::computeNeighborRightHandSideContribution()
{
  return -computeFluxRHSContribution();
}

Real
LinearFVAnisotropicDiffusion::computeFluxMatrixContribution()
{
  // If we don't have the value yet, we compute it
  if (!_cached_matrix_contribution)
  {
    const auto face_arg = makeCDFace(*_current_face_info);

    // If we requested nonorthogonal correction, we use the normal component of the
    // cell to face vector.
    const auto d = _use_nonorthogonal_correction
                       ? std::abs(_current_face_info->dCN() * _current_face_info->normal())
                       : _current_face_info->dCNMag();

    auto scaled_diff_tensor = _diffusion_tensor(face_arg, determineState());

    for (const auto i : make_range(Moose::dim))
      scaled_diff_tensor(i) = _current_face_info->normal()(i) * scaled_diff_tensor(i);

    auto normal_scaled_diff_tensor = scaled_diff_tensor * _current_face_info->normal();

    // Cache the matrix contribution
    _flux_matrix_contribution = normal_scaled_diff_tensor / d * _current_face_area;
    _cached_matrix_contribution = true;
  }

  return _flux_matrix_contribution;
}

Real
LinearFVAnisotropicDiffusion::computeFluxRHSContribution()
{
  // Cache the RHS contribution
  if (!_cached_rhs_contribution)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    const auto state_arg = determineState();

    // Get the gradients from the adjacent cells
    const auto grad_elem = _var.gradSln(*_current_face_info->elemInfo());
    const auto grad_neighbor = _var.gradSln(*_current_face_info->neighborInfo());

    // Interpolate the two gradients to the face
    const auto interp_coeffs =
        interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

    const auto interpolated_gradient =
        (interp_coeffs.first * grad_elem + interp_coeffs.second * grad_neighbor);

    auto scaled_diff_tensor = _diffusion_tensor(face_arg, state_arg);

    for (const auto i : make_range(Moose::dim))
      scaled_diff_tensor(i) = _current_face_info->normal()(i) * scaled_diff_tensor(i);

    auto normal_scaled_diff_tensor = scaled_diff_tensor * _current_face_info->normal();

    _flux_rhs_contribution =
        (scaled_diff_tensor - normal_scaled_diff_tensor * _current_face_info->normal()) *
        interpolated_gradient;

    if (_use_nonorthogonal_correction)
    {
      // Compute correction vector. Potential optimization: this only depends on the geometry
      // so we can cache it in FaceInfo at some point.
      const auto correction_vector =
          _current_face_info->normal() -
          1 / (_current_face_info->normal() * _current_face_info->eCN()) *
              _current_face_info->eCN();

      _flux_rhs_contribution +=
          normal_scaled_diff_tensor * interpolated_gradient * correction_vector;
    }
    _flux_rhs_contribution *= _current_face_area;
    _cached_rhs_contribution = true;
  }

  return _flux_rhs_contribution;
}

Real
LinearFVAnisotropicDiffusion::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(diff_bc, "This should be a valid BC!");

  auto grad_contrib = diff_bc->computeBoundaryGradientMatrixContribution() * _current_face_area;
  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!diff_bc->includesMaterialPropertyMultiplier())
  {
    const auto face_arg = singleSidedFaceArg(_current_face_info);

    auto scaled_diff_tensor = _diffusion_tensor(face_arg, determineState());

    for (const auto i : make_range(Moose::dim))
      scaled_diff_tensor(i) = _current_face_info->normal()(i) * scaled_diff_tensor(i);

    auto normal_scaled_diff_tensor = scaled_diff_tensor * _current_face_info->normal();

    grad_contrib *= normal_scaled_diff_tensor;
  }

  return grad_contrib;
}

Real
LinearFVAnisotropicDiffusion::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(diff_bc, "This should be a valid BC!");

  const auto face_arg = singleSidedFaceArg(_current_face_info);
  auto grad_contrib = diff_bc->computeBoundaryGradientRHSContribution();

  auto scaled_diff_tensor = _diffusion_tensor(face_arg, determineState());

  for (const auto i : make_range(Moose::dim))
    scaled_diff_tensor(i) = _current_face_info->normal()(i) * scaled_diff_tensor(i);

  auto normal_scaled_diff_tensor = scaled_diff_tensor * _current_face_info->normal();
  auto boundary_grad = _var.gradSln(*_current_face_info->elemInfo());

  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!diff_bc->includesMaterialPropertyMultiplier())
  {
    grad_contrib *= normal_scaled_diff_tensor;
  }

  grad_contrib += (scaled_diff_tensor - normal_scaled_diff_tensor * _current_face_info->normal()) *
                  boundary_grad;

  // We add the nonorthogonal corrector for the face here. Potential idea: we could do
  // this in the boundary condition too. For now, however, we keep it like this.
  if (_use_nonorthogonal_correction_on_boundary)
  {
    const auto correction_vector =
        _current_face_info->normal() -
        1 / (_current_face_info->normal() * _current_face_info->eCN()) * _current_face_info->eCN();

    grad_contrib += normal_scaled_diff_tensor * boundary_grad * correction_vector;
  }

  return grad_contrib * _current_face_area;
}
