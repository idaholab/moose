//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVScalarAdvection.h"

#include "LinearFVGradientManager.h"
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
  params.addParam<InterpolationMethodName>(
      "slip_advected_interp_method_name",
      "Scheme for the drift flux. The drift is always interpolated separately from the mixture "
      "flux, each upwinding in its own direction; this chooses which scheme the drift half uses. "
      "Leave it unset and the drift takes the same scheme as the mixture flux. Setting it is what "
      "allows a limiter to act on the drift correction alone, which is the treatment a bounded "
      "phase fraction needs at high dispersed phase fraction.");
  params.addParam<MooseFunctorName>(
      "density",
      "Density multiplying the advective flux, so that the conservative form div(rho phi u) is "
      "assembled instead of div(phi u). The matching 'factor' must be set on the time derivative "
      "kernel. A constant density then leaves the solution unchanged, because every term of the "
      "equation is scaled by the same number; that is only true once every other kernel on this "
      "variable carries the same factor, so a diffusion or source term left unscaled would "
      "change the answer.");
  params.addParam<std::vector<BoundaryName>>(
      "slip_boundaries",
      {},
      "Boundaries across which the dispersed phase may travel, normally the inlets and the "
      "outlets. The slip velocity contributes to the advective flux on these boundaries and "
      "on every internal face, but not on impermeable boundaries such as walls, where the "
      "phase cannot cross even though the advected variable may carry a boundary condition.");
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
    _gradient_field(_adv_interp_method.needsGradients()
                        ? &_var.requestCellGradients(_adv_interp_method.gradientMethodName())
                        : nullptr),
    _slip_adv_interp_method(getFVAdvectedInterpolationMethod(
        isParamValid("slip_advected_interp_method_name")
            ? getParam<InterpolationMethodName>("slip_advected_interp_method_name")
            : getParam<InterpolationMethodName>("advected_interp_method_name"))),
    _slip_gradient_field(
        _slip_adv_interp_method.needsGradients()
            ? &_var.requestCellGradients(_slip_adv_interp_method.gradientMethodName())
            : nullptr),
    _volumetric_face_flux(0.0),
    _slip_face_flux(0.0),
    _density(isParamValid("density") ? &getFunctor<Real>("density") : nullptr),
    _u_slip(isParamValid("u_slip") ? &getFunctor<ADReal>("u_slip") : nullptr),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr),
    _add_slip_model(isParamValid("u_slip") ? true : false)
{
  const auto slip_ids =
      _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("slip_boundaries"));
  _slip_boundaries.insert(slip_ids.begin(), slip_ids.end());
}

Real
LinearFVScalarAdvection::computeElemMatrixContribution()
{
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.first * _volumetric_face_flux * _current_face_area +
         _slip_interp_result.weights_matrix.first * _slip_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeNeighborMatrixContribution()
{
  const auto & coeffs = _adv_interp_result.weights_matrix;
  return coeffs.second * _volumetric_face_flux * _current_face_area +
         _slip_interp_result.weights_matrix.second * _slip_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeElemRightHandSideContribution()
{
  return _adv_interp_result.rhs_face_value * _volumetric_face_flux * _current_face_area +
         _slip_interp_result.rhs_face_value * _slip_face_flux * _current_face_area;
}

Real
LinearFVScalarAdvection::computeNeighborRightHandSideContribution()
{
  return -(_adv_interp_result.rhs_face_value * _volumetric_face_flux +
           _slip_interp_result.rhs_face_value * _slip_face_flux) *
         _current_face_area;
}

Real
LinearFVScalarAdvection::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = cast_ptr<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  const auto boundary_value_matrix_contrib = adv_bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * (_volumetric_face_flux + _slip_face_flux) *
         _current_face_area;
}

Real
LinearFVScalarAdvection::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_bc = cast_ptr<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_bc, "This should be a valid BC!");

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = adv_bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * (_volumetric_face_flux + _slip_face_flux) *
         _current_face_area;
}

void
LinearFVScalarAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the velocity on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _volumetric_face_flux = _mass_flux_provider.getVolumetricFaceFlux(*face_info);

  _slip_face_flux = 0.0;

  // The phase fraction is advected at the dispersed phase velocity u_m + u_Md, so the drift has to
  // reach the flux.
  //
  // The drift is added on every internal face, and on a boundary face only if that boundary is one
  // of the 'slip_boundaries', the inlets and outlets the phase enters and leaves through. On a wall
  // the mixture flux from the Rhie-Chow object is already zero, and a gravity driven drift added
  // there would advect phase straight through the wall, whether or not the boundary carries a
  // condition on the phase fraction: a wall may prescribe the fraction and still let no phase
  // through.
  if (_u_slip && NS::slipAllowedOnFace(*face_info, _slip_boundaries))
  {
    const auto state = determineState();
    // TODO Add boundary treatment to be able select two-term expansion if desired
    const auto face_arg = face_info->neighborPtr()
                              ? Moose::FaceArg{face_info,
                                               Moose::FV::LimiterType::CentralDifference,
                                               true,
                                               false,
                                               face_info->neighborPtr(),
                                               nullptr}
                              : singleSidedFaceArg(face_info);

    const RealVectorValue velocity_slip_vel_vec =
        MetaPhysicL::raw_value(NS::slipVelocityVector(*_u_slip, _v_slip, _w_slip, face_arg, state));

    // The drift is held apart from the mixture flux and interpolated on its own terms, so the
    // donor cell it selects is the one the drift itself points away from. Each part is then
    // separately in donor cell form, which is what keeps the pair an M-matrix.
    _slip_face_flux = velocity_slip_vel_vec * face_info->normal();
  }

  // Turn the volumetric fluxes into mass fluxes when a density was supplied. This is applied after
  // the slip so that the density multiplies the total phase velocity, and before the interpolation
  // below so that the upwind directions are unaffected by it, the density being positive.
  if (_density)
  {
    const auto face_arg =
        face_info->neighborPtr() ? makeCDFace(*face_info) : singleSidedFaceArg(face_info);
    const auto rho_face = (*_density)(face_arg, determineState());
    _volumetric_face_flux *= rho_face;
    _slip_face_flux *= rho_face;
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
    mooseAssert(_gradient_field, "Gradient field should be registered when gradients are needed.");
    _elem_grad_storage = _gradient_field->gradient(elem_info);
    _neighbor_grad_storage = _gradient_field->gradient(neighbor_info);
  }

  _adv_interp_result = _adv_interp_method.advectedInterpolate(*_current_face_info,
                                                              elem_value,
                                                              neighbor_value,
                                                              &_elem_grad_storage,
                                                              &_neighbor_grad_storage,
                                                              _volumetric_face_flux);

  // The drift flux is interpolated on its own terms, against its own scheme. The flux handed over
  // is the drift alone, so the donor cell it selects is the one the drift itself points away from.
  if (_slip_adv_interp_method.needsGradients())
  {
    mooseAssert(_slip_gradient_field,
                "Gradient field should be registered when gradients are needed.");
    _slip_elem_grad_storage = _slip_gradient_field->gradient(elem_info);
    _slip_neighbor_grad_storage = _slip_gradient_field->gradient(neighbor_info);
  }

  _slip_interp_result = _slip_adv_interp_method.advectedInterpolate(*_current_face_info,
                                                                    elem_value,
                                                                    neighbor_value,
                                                                    &_slip_elem_grad_storage,
                                                                    &_slip_neighbor_grad_storage,
                                                                    _slip_face_flux);
}
