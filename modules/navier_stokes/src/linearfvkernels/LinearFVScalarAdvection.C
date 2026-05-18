//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVScalarAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVScalarAdvection);

InputParameters
LinearFVScalarAdvection::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for a passive scalar.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params.addRequiredParam<InterpolationMethodName>(
      "advected_interp_method_name",
      "Name of the FVInterpolationMethod to use for the advected quantity.");
  params.addParam<MooseFunctorName>("u_slip", "The slip-velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip-velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip-velocity in the z direction.");
  return params;
}

LinearFVScalarAdvection::LinearFVScalarAdvection(const InputParameters & params)
  : LinearFVFluxKernel(params),
    FVInterpolationMethodInterface(this),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _adv_interp_method(getFVAdvectedInterpolationMethod(
        getParam<InterpolationMethodName>("advected_interp_method_name"))),
    _volumetric_face_flux(0.0),
    _u_slip(isParamValid("u_slip") ? &getFunctor<ADReal>("u_slip") : nullptr),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr),
    _add_slip_model(isParamValid("u_slip") ? true : false)
{
  if (_adv_interp_method.needsGradients())
    _var.computeCellGradients(_adv_interp_method.gradientLimiter());
}

Real
LinearFVScalarAdvection::computeElemMatrixContribution()
{
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.first * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeNeighborMatrixContribution()
{
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.second * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeElemRightHandSideContribution()
{
  return _adv_interp_result.rhs_face_value * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeNeighborRightHandSideContribution()
{
  return -_adv_interp_result.rhs_face_value * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * _volumetric_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * _volumetric_face_flux * _current_face_area;
}

void
LinearFVScalarAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the velocity on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _volumetric_face_flux = _mass_flux_provider.getVolumetricFaceFlux(*face_info);

  // Adjust volumetric face flux using the slip velocity
  // TODO: add boundaries
  if (_u_slip && face_info->neighborPtr())
  {
    const auto state = determineState();
    Moose::FaceArg face_arg;
    // TODO Add boundary treatment to be able select two-term expansion if desired
    face_arg = Moose::FaceArg{face_info,
                              Moose::FV::LimiterType::CentralDifference,
                              true,
                              false,
                              face_info->neighborPtr(),
                              nullptr};

    RealVectorValue velocity_slip_vel_vec;
    if (_u_slip)
      velocity_slip_vel_vec(0) = (*_u_slip)(face_arg, state).value();
    if (_v_slip)
      velocity_slip_vel_vec(1) = (*_v_slip)(face_arg, state).value();
    if (_w_slip)
      velocity_slip_vel_vec(2) = (*_w_slip)(face_arg, state).value();
    _volumetric_face_flux += velocity_slip_vel_vec * face_info->normal();
  }

  // Only internal faces need advected interpolation results; boundary contributions are handled
  // through the linear FV boundary conditions.
  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return;

  const auto state = determineState();
  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();

  const Real elem_value = _var.getElemValue(elem_info, state);
  const Real neighbor_value = _var.getElemValue(neighbor_info, state);
  if (_adv_interp_method.needsGradients())
  {
    const auto limiter_type = _adv_interp_method.gradientLimiter();
    _elem_grad_storage = _var.gradSln(elem_info, state, limiter_type);
    _neighbor_grad_storage = _var.gradSln(neighbor_info, state, limiter_type);
  }

  _adv_interp_result = _adv_interp_method.advectedInterpolate(*_current_face_info,
                                                              elem_value,
                                                              neighbor_value,
                                                              &_elem_grad_storage,
                                                              &_neighbor_grad_storage,
                                                              _volumetric_face_flux);
}
