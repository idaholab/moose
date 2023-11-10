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

registerMooseObject("MooseApp", LinearFVDiffusionKernel);

InputParameters
LinearFVDiffusionKernel::validParams()
{
  InputParameters params = FluxLinearFVKernel::validParams();
  params.addParam<Real>("diffusion_coeff", 1.0, "The reaction coefficient.");
  return params;
}

LinearFVDiffusionKernel::LinearFVDiffusionKernel(const InputParameters & params)
  : FluxLinearFVKernel(params), _diffusion_coeff(getParam<Real>("diffusion_coeff"))
{
}

Real
LinearFVDiffusionKernel::computeElemMatrixContribution()
{
  return _diffusion_coeff * _current_face_info->gC() * _current_face_info->faceArea() *
         _current_face_info->faceCoord();
}

Real
LinearFVDiffusionKernel::computeNeighborMatrixContribution()
{
  return _diffusion_coeff * (1 - _current_face_info->gC()) * _current_face_info->faceArea() *
         _current_face_info->faceCoord();
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
