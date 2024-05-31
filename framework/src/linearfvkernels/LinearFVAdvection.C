//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvection.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("MooseApp", LinearFVAdvection);

InputParameters
LinearFVAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term in a partial differential equation.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params += Moose::FV::advectedInterpolationParameter();
  return params;
}

LinearFVAdvection::LinearFVAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params), _velocity(getParam<RealVectorValue>("velocity"))

{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

void
LinearFVAdvection::initialSetup()
{
  for (const auto bc : _var.getBoundaryConditionMap())
    if (!dynamic_cast<const LinearFVAdvectionDiffusionBC *>(bc.second))
      mooseError(
          bc.second->type(), " is not a compatible boundary condition with ", this->type(), "!");
}

Real
LinearFVAdvection::computeElemMatrixContribution()
{
  const Real face_flux = _velocity * _current_face_info->normal();
  const auto interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, face_flux);
  return interp_coeffs.first * face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeNeighborMatrixContribution()
{
  const Real face_flux = _velocity * _current_face_info->normal();
  const auto interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, face_flux);
  return interp_coeffs.second * face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeElemRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVAdvection::computeNeighborRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * (_velocity * _current_face_info->normal()) *
         _current_face_area;
}

Real
LinearFVAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * (_velocity * _current_face_info->normal()) *
         _current_face_area;
}
