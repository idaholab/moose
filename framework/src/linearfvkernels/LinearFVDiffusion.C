//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDiffusion.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("MooseApp", LinearFVDiffusion);

InputParameters
LinearFVDiffusion::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of a "
                             "diffusion term in a partial differential equation.");
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "If the nonorthogonal correction should be used when computing the normal gradient.");
  params.addParam<MooseFunctorName>("diffusion_coeff", 1.0, "The diffusion coefficient.");
  return params;
}

LinearFVDiffusion::LinearFVDiffusion(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _diffusion_coeff(getFunctor<Real>("diffusion_coeff")),
    _use_nonorthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _flux_matrix_contribution(0.0),
    _flux_rhs_contribution(0.0)
{
  if (_use_nonorthogonal_correction)
    _var.computeCellGradients();
}

void
LinearFVDiffusion::initialSetup()
{
  for (const auto bc : _var.getBoundaryConditionMap())
    if (!dynamic_cast<const LinearFVAdvectionDiffusionBC *>(bc.second))
      mooseError(
          bc.second->type(), " is not a compatible boundary condition with ", this->type(), "!");
}

Real
LinearFVDiffusion::computeElemMatrixContribution()
{
  return computeFluxMatrixContribution();
}

Real
LinearFVDiffusion::computeNeighborMatrixContribution()
{
  return -computeFluxMatrixContribution();
}

Real
LinearFVDiffusion::computeElemRightHandSideContribution()
{
  return computeFluxRHSContribution();
}

Real
LinearFVDiffusion::computeNeighborRightHandSideContribution()
{
  return -computeFluxRHSContribution();
}

Real
LinearFVDiffusion::computeFluxMatrixContribution()
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

    // Cache the matrix contribution
    _flux_matrix_contribution =
        _diffusion_coeff(face_arg, determineState()) / d * _current_face_area;
    _cached_matrix_contribution = true;
  }

  return _flux_matrix_contribution;
}

Real
LinearFVDiffusion::computeFluxRHSContribution()
{
  // We only have contributions on the right hand side from internal faces
  // if the nonorthogonal correction is enabled.
  if (_use_nonorthogonal_correction && !_cached_rhs_contribution)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    const auto state_arg = determineState();

    // Get the gradients from the adjacent cells
    const auto grad_elem = _var.gradSln(*_current_face_info->elemInfo());
    const auto & grad_neighbor = _var.gradSln(*_current_face_info->neighborInfo());

    // Interpolate the two gradients to the face
    const auto interp_coeffs =
        interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

    // Compute correction vector. Potential optimization: this only depends on the geometry
    // so we can cache it in FaceInfo at some point.
    const auto correction_vector =
        _current_face_info->normal() -
        1 / (_current_face_info->normal() * _current_face_info->eCN()) * _current_face_info->eCN();

    // Cache the matrix contribution
    _flux_rhs_contribution =
        _diffusion_coeff(face_arg, state_arg) *
        (interp_coeffs.first * grad_elem + interp_coeffs.second * grad_neighbor) *
        correction_vector * _current_face_area;
    _cached_rhs_contribution = true;
  }

  return _flux_rhs_contribution;
}

Real
LinearFVDiffusion::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(diff_bc, "This should be a valid BC!");

  auto grad_contrib = diff_bc->computeBoundaryGradientMatrixContribution() * _current_face_area;
  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!diff_bc->includesMaterialPropertyMultiplier())
  {
    const auto face_arg = singleSidedFaceArg(_current_face_info);
    grad_contrib *= _diffusion_coeff(face_arg, determineState());
  }

  return grad_contrib;
}

Real
LinearFVDiffusion::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(diff_bc, "This should be a valid BC!");

  const auto face_arg = singleSidedFaceArg(_current_face_info);
  auto grad_contrib = diff_bc->computeBoundaryGradientRHSContribution() * _current_face_area;

  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!diff_bc->includesMaterialPropertyMultiplier())
    grad_contrib *= _diffusion_coeff(face_arg, determineState());

  // We add the nonorthogonal corrector for the face here. Potential idea: we could do
  // this in the boundary condition too. For now, however, we keep it like this.
  // This should only be used for BCs where the gradient of the value is computed and
  // not prescribed.

  if (_use_nonorthogonal_correction && diff_bc->useBoundaryGradientExtrapolation())
  {
    // We support internal boundaries as well. In that case we have to decide on which side
    // of the boundary we are on.
    const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
    const Real boundary_normal_multiplier =
        (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

    // Unit vector to the boundary. Unfortunately, we have to recompute it because the value
    // stored in the face info is only correct for external boundaries
    const auto e_Cf = _current_face_info->faceCentroid() - elem_info->centroid();
    const auto correction_vector =
        _current_face_info->normal() - 1 / (_current_face_info->normal() * e_Cf) * e_Cf;

    grad_contrib += _diffusion_coeff(face_arg, determineState()) * _var.gradSln(*elem_info) *
                    boundary_normal_multiplier * correction_vector * _current_face_area;
  }

  return grad_contrib;
}
