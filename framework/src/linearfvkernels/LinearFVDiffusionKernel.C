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
  params.addParam<MooseFunctorName>("diffusion_coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVDiffusionKernel::LinearFVDiffusionKernel(const InputParameters & params)
  : LinearFVFluxKernel(params), _diffusion_coeff(getFunctor<Real>("diffusion_coeff"))
{
}

Real
LinearFVDiffusionKernel::computeElemMatrixContribution()
{
  const auto face_arg = makeCDFace(*_current_face_info);
  return _diffusion_coeff(face_arg, determineState()) / _current_face_info->dCNMag() *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}

Real
LinearFVDiffusionKernel::computeNeighborMatrixContribution()
{
  const auto face_arg = makeCDFace(*_current_face_info);
  return -_diffusion_coeff(face_arg, determineState()) / _current_face_info->dCNMag() *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}

Real
LinearFVDiffusionKernel::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVDiffusionKernel::computeNeighborRightHandSideContribution()
{
  return 0.0;
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
  auto grad_contrib = bc->computeBoundaryGradientRHSContribution();
  if (!bc->includesMaterialPropertyMultiplier())
  {
    const auto face_arg = singleSidedFaceArg(_current_face_info);
    grad_contrib *= _diffusion_coeff(face_arg, determineState());
  }

  return grad_contrib;
}
