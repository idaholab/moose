//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredParam<InterpolationMethodName>(
      "advected_interp_method_name",
      "Name of the FVInterpolationMethod to use for the advected quantity.");
  return params;
}

LinearFVAdvection::LinearFVAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    FVInterpolationMethodInterface(this),
    _velocity(getParam<RealVectorValue>("velocity")),
    _adv_interp_method(
        &getFVInterpolationMethod(getParam<InterpolationMethodName>("advected_interp_method_name")))

{
  mooseAssert(_adv_interp_method,
              "LinearFVAdvection now requires an explicit FVInterpolationMethod "
              "via advected_interp_method_name.");

  if (!_adv_interp_method->supportsAdvectedInterpolation())
    mooseError("FVInterpolationMethod '",
               _adv_interp_method->name(),
               "' (",
               _adv_interp_method->type(),
               ") does not support advected interpolation and cannot be used by ",
               type(),
               ".");

  if (_adv_interp_method->advectedInterpolationNeedsGradients())
    _var.computeCellGradients(_adv_interp_method->gradientLimiter());
}

void
LinearFVAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);
  _adv_face_flux = _velocity * _current_face_info->normal();

  // Only internal faces need advected interpolation results; boundary contributions are handled
  // through the linear FV boundary conditions.
  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return;

  const auto state = determineState();
  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();

  const Real elem_value = _var.getElemValue(elem_info, state);
  const Real neighbor_value = _var.getElemValue(neighbor_info, state);
  if (_adv_interp_method->advectedInterpolationNeedsGradients())
  {
    const auto limiter_type = _adv_interp_method->gradientLimiter();
    _elem_grad_storage = _var.gradSln(elem_info, limiter_type);
    _neighbor_grad_storage = _var.gradSln(neighbor_info, limiter_type);
  }

  _adv_interp_result = _adv_interp_method->advectedInterpolate(*_current_face_info,
                                                               elem_value,
                                                               neighbor_value,
                                                               &_elem_grad_storage,
                                                               &_neighbor_grad_storage,
                                                               _adv_face_flux);
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
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.first * _adv_face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeNeighborMatrixContribution()
{
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.second * _adv_face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeElemRightHandSideContribution()
{
  return _adv_interp_result.rhs_face_value * _adv_face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeNeighborRightHandSideContribution()
{
  return -_adv_interp_result.rhs_face_value * _adv_face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * _adv_face_flux * _current_face_area;
}

Real
LinearFVAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * _adv_face_flux * _current_face_area;
}
