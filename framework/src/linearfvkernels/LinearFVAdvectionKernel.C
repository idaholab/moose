//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionKernel.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVBoundaryCondition.h"

registerMooseObject("MooseApp", LinearFVAdvectionKernel);

InputParameters
LinearFVAdvectionKernel::validParams()
{
  InputParameters params = FluxLinearFVKernel::validParams();
  params.addRequiredParam<Real>("density", "Constant density for advected quantity multiplication");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVAdvectionKernel::LinearFVAdvectionKernel(const InputParameters & params)
  : FluxLinearFVKernel(params),
    _density(getParam<Real>("density")),
    _velocity(getParam<RealVectorValue>("velocity"))
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

Real
LinearFVAdvectionKernel::computeElemMatrixContribution()
{
  const auto interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _velocity);
  return interp_coeffs.first * _density * (_velocity * _current_face_info->normal()) *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}

Real
LinearFVAdvectionKernel::computeNeighborMatrixContribution()
{
  const auto interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _velocity);
  return interp_coeffs.second * _density * (_velocity * _current_face_info->normal()) *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}

Real
LinearFVAdvectionKernel::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVAdvectionKernel::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVAdvectionKernel::computeBoundaryMatrixContribution()
{
  mooseAssert(_current_face_info->boundaryIDs().size() == 1,
              "We should only have one boundary on every face.");
  const auto * boundary_condition =
      _var->getBoundaryCondition(*_current_face_info->boundaryIDs().begin());
  auto value_contrib =
      boundary_condition->computeBoundaryValueMatrixContribution(_current_face_info);

  return value_contrib * _density * (_velocity * _current_face_info->normal()) *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}

Real
LinearFVAdvectionKernel::computeBoundaryRHSContribution()
{
  mooseAssert(_current_face_info->boundaryIDs().size() == 1,
              "We should only have one boundary on every face.");

  const auto * boundary_condition =
      _var->getBoundaryCondition(*_current_face_info->boundaryIDs().begin());
  auto value_contrib = boundary_condition->computeBoundaryValueRHSContribution(_current_face_info);

  return -value_contrib * _density * (_velocity * _current_face_info->normal()) *
         _current_face_info->faceArea() * _current_face_info->faceCoord();
}
