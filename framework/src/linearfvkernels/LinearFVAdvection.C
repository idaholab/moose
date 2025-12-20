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
#include <limits>

registerMooseObject("MooseApp", LinearFVAdvection);

InputParameters
LinearFVAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term in a partial differential equation.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params += Moose::FV::advectedInterpolationParameter();
  params.addParam<InterpolationMethodName>(
      "advected_interp_method_name",
      "Optional FVInterpolationMethod to use for the advected quantity. If provided, this "
      "overrides "
      "the advected_interp_method enum and enables schemes like TVD vanLeer.");
  return params;
}

LinearFVAdvection::LinearFVAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    FVInterpolationMethodInterface(this),
    _velocity(getParam<RealVectorValue>("velocity")),
    _adv_interp_method(isParamValid("advected_interp_method_name")
                           ? &getFVInterpolationMethod(
                                 getParam<InterpolationMethodName>("advected_interp_method_name"))
                           : nullptr)

{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");

  if (_adv_interp_method)
  {
    _adv_interp_handle = _adv_interp_method->advectedFaceInterpolator();
    if (_adv_interp_handle.needsGradients())
      _var.computeCellGradients();
  }
}

const LinearFVAdvection::AdvectedCacheEntry &
LinearFVAdvection::computeAdvectedWeights(const Real face_flux)
{
  const auto face_id = _current_face_info->id();
  auto & entry = _adv_interp_cache[face_id];

  const auto state = determineState();
  const auto elem_value = _var.getElemValue(*_current_face_info->elemInfo(), state);
  const auto neighbor_value = _var.getElemValue(*_current_face_info->neighborInfo(), state);
  const VectorValue<Real> * elem_grad = _adv_interp_handle.needsGradients()
                                            ? &_var.gradSln(*_current_face_info->elemInfo())
                                            : nullptr;
  const VectorValue<Real> * neighbor_grad = _adv_interp_handle.needsGradients()
                                                ? &_var.gradSln(*_current_face_info->neighborInfo())
                                                : nullptr;

  // Interpolation result from advected method (may include deferred correction intent)
  entry.result = _adv_interp_handle(
      *_current_face_info, elem_value, neighbor_value, elem_grad, neighbor_grad, face_flux);

  // Deferred correction flux based on current solution and weights provided by the method
  const auto & wm = entry.result.weights_matrix;
  const auto & wh = entry.result.weights_high;
  const Real face_matrix = wm.first * elem_value + wm.second * neighbor_value;
  const Real face_high = wh.first * elem_value + wh.second * neighbor_value;
  entry.correction_flux = entry.result.has_correction
                              ? (face_high - face_matrix) * face_flux * _current_face_area
                              : 0.0;
  return entry;
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

  if (_adv_interp_handle.valid())
  {
    const auto & cache = computeAdvectedWeights(face_flux);
    const auto coeffs = cache.result.weights_matrix;
    return coeffs.first * face_flux * _current_face_area;
  }
  else
  {
    const auto interp_coeffs =
        interpCoeffs(_advected_interp_method, *_current_face_info, true, face_flux);
    return interp_coeffs.first * face_flux * _current_face_area;
  }
}

Real
LinearFVAdvection::computeNeighborMatrixContribution()
{
  const Real face_flux = _velocity * _current_face_info->normal();

  if (_adv_interp_handle.valid())
  {
    const auto & cache = computeAdvectedWeights(face_flux);
    const auto coeffs = cache.result.weights_matrix;
    return coeffs.second * face_flux * _current_face_area;
  }
  else
  {
    const auto interp_coeffs =
        interpCoeffs(_advected_interp_method, *_current_face_info, true, face_flux);
    return interp_coeffs.second * face_flux * _current_face_area;
  }
}

Real
LinearFVAdvection::computeElemRightHandSideContribution()
{
  if (_adv_interp_handle.valid())
  {
    const Real face_flux = _velocity * _current_face_info->normal();
    const auto & cache = computeAdvectedWeights(face_flux);
    return cache.correction_flux;
  }
  return 0.0;
}

Real
LinearFVAdvection::computeNeighborRightHandSideContribution()
{
  if (_adv_interp_handle.valid())
  {
    const Real face_flux = _velocity * _current_face_info->normal();
    const auto & cache = computeAdvectedWeights(face_flux);
    return -cache.correction_flux;
  }
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
