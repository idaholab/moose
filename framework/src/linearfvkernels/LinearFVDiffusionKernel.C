//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVDiffusionKernel.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVBoundaryCondition.h"

registerMooseObject("MooseApp", LinearFVDiffusionKernel);

InputParameters
LinearFVDiffusionKernel::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of a "
                             "diffusion term in a partial differential equation.");
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "If the nonorthogonal correction should be used when computing the normal gradient.");
  params.addParam<MooseFunctorName>("diffusion_coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVDiffusionKernel::LinearFVDiffusionKernel(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _diffusion_coeff(getFunctor<Real>("diffusion_coeff")),
    _use_nonorthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _flux_matrix_contribution(0.0),
    _flux_rhs_contribution(0.0)
{
  if (_use_nonorthogonal_correction)
    _var->computeCellGradients();
}

Real
LinearFVDiffusionKernel::computeElemMatrixContribution()
{
  return computeFluxMatrixContribution();
}

Real
LinearFVDiffusionKernel::computeNeighborMatrixContribution()
{
  return -computeFluxMatrixContribution();
}

Real
LinearFVDiffusionKernel::computeElemRightHandSideContribution()
{
  return computeFluxRHSContribution();
}

Real
LinearFVDiffusionKernel::computeNeighborRightHandSideContribution()
{
  return -computeFluxRHSContribution();
}

Real
LinearFVDiffusionKernel::computeFluxMatrixContribution()
{
  if (!_cached_matrix_contribution)
  {
    const auto face_arg = makeCDFace(*_current_face_info);
    const auto d = _use_nonorthogonal_correction
                       ? std::abs(_current_face_info->dCN() * _current_face_info->normal())
                       : _current_face_info->dCNMag();
    _flux_matrix_contribution = _diffusion_coeff(face_arg, determineState()) / d *
                                _current_face_info->faceArea() * _current_face_info->faceCoord();
    _cached_matrix_contribution = true;
  }

  return _flux_matrix_contribution;
}

Real
LinearFVDiffusionKernel::computeFluxRHSContribution()
{
  if (_use_nonorthogonal_correction)
    if (!_cached_rhs_contribution)
    {
      const auto face_arg = makeCDFace(*_current_face_info);
      const auto grad_elem = _var->gradSln(_current_face_info->elemInfo());
      const auto & grad_neighbor = _var->gradSln(_current_face_info->neighborInfo());

      const auto interp_coeffs = interpCoeffs(
          Moose::FV::InterpMethod::Average, *_current_face_info, true, RealVectorValue(0));

      const auto correction_vector =
          _current_face_info->normal() -
          1 / (_current_face_info->normal() * _current_face_info->eCN()) *
              _current_face_info->eCN();

      _flux_rhs_contribution =
          _diffusion_coeff(face_arg, determineState()) *
          (interp_coeffs.first * grad_elem + interp_coeffs.second * grad_neighbor) *
          correction_vector * _current_face_info->faceArea() * _current_face_info->faceCoord();
      _cached_rhs_contribution = true;
    }

  return _flux_rhs_contribution;
}

Real
LinearFVDiffusionKernel::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition * bc)
{
  auto grad_contrib = bc->computeBoundaryGradientMatrixContribution();
  if (!bc->includesMaterialPropertyMultiplier())
  {
    const auto face_arg = singleSidedFaceArg(_current_face_info);
    grad_contrib *= _diffusion_coeff(face_arg, determineState());
  }

  return grad_contrib;
}

Real
LinearFVDiffusionKernel::computeBoundaryRHSContribution(const LinearFVBoundaryCondition * bc)
{
  const auto face_arg = singleSidedFaceArg(_current_face_info);
  auto grad_contrib = bc->computeBoundaryGradientRHSContribution();
  if (!bc->includesMaterialPropertyMultiplier())
    grad_contrib *= _diffusion_coeff(face_arg, determineState());

  if (_use_nonorthogonal_correction)
  {
    const auto correction_vector =
        _current_face_info->normal() -
        1 / (_current_face_info->normal() * _current_face_info->eCN()) * _current_face_info->eCN();

    grad_contrib += _diffusion_coeff(face_arg, determineState()) *
                    _var->gradSln(_current_face_info->elemInfo()) * correction_vector *
                    _current_face_info->faceArea() * _current_face_info->faceCoord();
  }

  return grad_contrib;
}
