//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFV2PScalarAdvection.h"
#include "LinearFVGradientManager.h"
#include "MooseLinearVariableFV.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearWCNSFV2PScalarAdvection);

InputParameters
LinearWCNSFV2PScalarAdvection::validParams()
{
  InputParameters params = LinearFVScalarAdvection::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of the advection of the dispersed "
      "phase fraction of a two-phase mixture, carried by the mixture flux and by the drift of the "
      "dispersed phase relative to the mixture.");
  params.addParam<InterpolationMethodName>(
      "slip_advected_interp_method_name",
      "Scheme for the drift flux. The drift is always interpolated separately from the mixture "
      "flux, each upwinding in its own direction; this chooses which scheme the drift half uses. "
      "Leave it unset and the drift takes the same scheme as the mixture flux. Setting it is what "
      "allows a limiter to act on the drift correction alone, which is the treatment a bounded "
      "phase fraction needs at high dispersed phase fraction.");
  params.addParam<std::vector<BoundaryName>>(
      "slip_boundaries",
      {},
      "Boundaries across which the dispersed phase may travel, normally the inlets and the "
      "outlets. The slip velocity contributes to the advective flux on these boundaries and "
      "on every internal face, but not on impermeable boundaries such as walls, where the "
      "phase cannot cross even though the advected variable may carry a boundary condition.");
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip-velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip-velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip-velocity in the z direction.");
  return params;
}

LinearWCNSFV2PScalarAdvection::LinearWCNSFV2PScalarAdvection(const InputParameters & params)
  : LinearFVScalarAdvection(params),
    _slip_adv_interp_method(getFVAdvectedInterpolationMethod(
        isParamValid("slip_advected_interp_method_name")
            ? getParam<InterpolationMethodName>("slip_advected_interp_method_name")
            : getParam<InterpolationMethodName>("advected_interp_method_name"))),
    _slip_gradient_field(
        _slip_adv_interp_method.needsGradients()
            ? &_var.requestCellGradients(_slip_adv_interp_method.gradientMethodName())
            : nullptr),
    _slip_face_flux(0.0),
    _u_slip(getFunctor<ADReal>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<ADReal>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<ADReal>("w_slip") : nullptr)
{
  const auto slip_ids =
      _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("slip_boundaries"));
  _slip_boundaries.insert(slip_ids.begin(), slip_ids.end());
}

Real
LinearWCNSFV2PScalarAdvection::computeElemMatrixContribution()
{
  return LinearFVScalarAdvection::computeElemMatrixContribution() +
         _slip_interp_result.weights_matrix.first * _slip_face_flux * _current_face_area;
}

Real
LinearWCNSFV2PScalarAdvection::computeNeighborMatrixContribution()
{
  return LinearFVScalarAdvection::computeNeighborMatrixContribution() +
         _slip_interp_result.weights_matrix.second * _slip_face_flux * _current_face_area;
}

Real
LinearWCNSFV2PScalarAdvection::computeElemRightHandSideContribution()
{
  return LinearFVScalarAdvection::computeElemRightHandSideContribution() +
         _slip_interp_result.rhs_face_value * _slip_face_flux * _current_face_area;
}

Real
LinearWCNSFV2PScalarAdvection::computeNeighborRightHandSideContribution()
{
  return LinearFVScalarAdvection::computeNeighborRightHandSideContribution() -
         _slip_interp_result.rhs_face_value * _slip_face_flux * _current_face_area;
}

void
LinearWCNSFV2PScalarAdvection::setupFaceData(const FaceInfo * face_info)
{
  LinearFVScalarAdvection::setupFaceData(face_info);

  _slip_face_flux = 0.0;

  // The phase fraction is advected at the dispersed phase velocity u_m + u_Md, so the drift has to
  // reach the flux.
  //
  // The drift is added on every face the phase fraction lives on from both sides, and on a
  // boundary face, including one internal to the mesh, only if that boundary is one of the
  // 'slip_boundaries', the inlets and outlets the phase enters and leaves through. On a wall the
  // mixture flux from the Rhie-Chow object is already zero, and a gravity driven drift added there
  // would advect phase straight through the wall, whether or not the boundary carries a condition
  // on the phase fraction: a wall may prescribe the fraction and still let no phase through.
  if (!NS::slipAllowedOnFace(*face_info, _current_face_type, _slip_boundaries))
    return;

  const auto state = determineState();
  // The same face argument as the drift flux kernels, so that the phase and the momentum and
  // energy drift fluxes see the same slip velocity on a face. On a boundary it is evaluated from
  // the side the phase fraction lives on.
  // TODO Add boundary treatment to be able select two-term expansion if desired
  const auto face_arg = (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
                            ? makeCDFace(*face_info)
                            : singleSidedFaceArg(face_info);

  const RealVectorValue velocity_slip_vel_vec =
      MetaPhysicL::raw_value(NS::slipVelocityVector(_u_slip, _v_slip, _w_slip, face_arg, state));

  // The drift is held apart from the mixture flux and interpolated on its own terms, so the
  // donor cell it selects is the one the drift itself points away from. Each part is then
  // separately in donor cell form, which is what keeps the pair an M-matrix.
  _slip_face_flux = velocity_slip_vel_vec * face_info->normal();

  // The same density as the mixture flux, so that it multiplies the total phase velocity
  if (_density)
    _slip_face_flux *= _face_density;

  // Only internal faces need advected interpolation results; boundary contributions are handled
  // through the linear FV boundary conditions.
  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
    return;

  const auto & elem_info = *_current_face_info->elemInfo();
  const auto & neighbor_info = *_current_face_info->neighborInfo();

  // The drift flux is interpolated on its own terms, against its own scheme. The flux handed over
  // is the drift alone, so the donor cell it selects is the one the drift itself points away from.
  if (_slip_adv_interp_method.needsGradients())
  {
    mooseAssert(_slip_gradient_field,
                "Gradient field should be registered when gradients are needed.");
    _slip_elem_grad_storage = _slip_gradient_field->gradient(elem_info);
    _slip_neighbor_grad_storage = _slip_gradient_field->gradient(neighbor_info);
  }

  _slip_interp_result =
      _slip_adv_interp_method.advectedInterpolate(*_current_face_info,
                                                  _var.getElemValue(elem_info, state),
                                                  _var.getElemValue(neighbor_info, state),
                                                  &_slip_elem_grad_storage,
                                                  &_slip_neighbor_grad_storage,
                                                  _slip_face_flux);
}
