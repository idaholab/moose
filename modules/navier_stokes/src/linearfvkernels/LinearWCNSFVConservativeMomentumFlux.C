//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVConservativeMomentumFlux.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NS.h"
#include "RhieChowMassFlux.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionBC.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVConservativeMomentumFlux);

InputParameters
LinearWCNSFVConservativeMomentumFlux::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription(
      "Represents the matrix and right hand side contributions of the "
      "stress and advection terms of an reference-solver-style velocity momentum "
      "equation. The historical object name is retained for the single "
      "sharp-interface parity path.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
  params.addParam<MooseFunctorName>(
      "mass_flux_functor",
      "Optional face-centered mass-flux functor used for momentum advection. When supplied, "
      "this overrides the Rhie-Chow user object's live face-mass-flux query so one convective "
      "flux family can be frozen for the whole outer iteration.");
  params.addParam<bool>(
      "require_mass_flux_functor",
      true,
      "Require mass_flux_functor instead of silently falling back to the live Rhie-Chow mass flux. "
      "The conservative sharp-interface parity path should consume the alpha-owned rhoPhi flux.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The diffusion coefficient.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<bool>(
      "use_nonorthogonal_correction",
      true,
      "If the nonorthogonal correction should be used when computing the normal gradient.");
  params.addParam<bool>(
      "use_deviatoric_terms", false, "If deviatoric terms in the stress terms need to be used.");

  params += Moose::FV::advectedInterpolationParameter();
  params.addParam<InterpolationMethodName>(
      "advected_interp_method_name",
      InterpolationMethodName(),
      "Optional FVInterpolationMethod object name for internal-face momentum advection. "
      "When provided, this supplies the matrix/RHS split for deferred-correction momentum "
      "advection and overrides the enum-only internal higher-order path.");
  return params;
}

LinearWCNSFVConservativeMomentumFlux::LinearWCNSFVConservativeMomentumFlux(
    const InputParameters & params)
  : LinearFVFluxKernel(params),
    FVInterpolationMethodInterface(this),
    _dim(_subproblem.mesh().dimension()),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _mass_flux_functor(params.isParamValid("mass_flux_functor")
                           ? &getFunctor<Real>("mass_flux_functor")
                           : nullptr),
    _require_mass_flux_functor(getParam<bool>("require_mass_flux_functor")),
    _mu(getFunctor<Real>(getParam<MooseFunctorName>(NS::mu))),
    _use_nonorthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _use_deviatoric_terms(getParam<bool>("use_deviatoric_terms")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _advected_rhs_face_value(0.0),
    _face_mass_flux(0.0),
    _boundary_normal_factor(1.0),
    _stress_matrix_contribution(0.0),
    _stress_rhs_contribution(0.0),
    _adv_interp_method(getParam<InterpolationMethodName>("advected_interp_method_name").empty()
                           ? nullptr
                           : &getFVAdvectedInterpolationMethod(
                                 getParam<InterpolationMethodName>("advected_interp_method_name"))),
    _index(getParam<MooseEnum>("momentum_component")),
    _velocity_vars{nullptr, nullptr, nullptr},
    _coord_type(getBlockCoordSystem()),
    _rz_radial_coord(_fe_problem.mesh().getAxisymmetricRadialCoord())
{
  if (_require_mass_flux_functor && !_mass_flux_functor)
    paramError("mass_flux_functor",
               "LinearWCNSFVConservativeMomentumFlux requires an alpha-owned rhoPhi mass-flux "
               "functor. This avoids falling back to rho_f * phi in the sharp-interface parity "
               "path.");

  // We only need gradients if the nonorthogonal correction is enabled or when we request the
  // computation of the deviatoric parts of the stress tensor.
  if (_use_nonorthogonal_correction || _use_deviatoric_terms)
    _var.computeCellGradients();

  const bool need_more_ghosting =
      Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
  if (_adv_interp_method)
  {
    if (_adv_interp_method->needsGradients())
      _var.computeCellGradients(_adv_interp_method->gradientLimiter());
  }
  else if (need_more_ghosting)
    paramError(
        "advected_interp_method",
        "Higher-order momentum advection requires a deferred-correction interpolation method "
        "object. Use the flow-physics provided Venkatakrishnan deferred-correction path rather "
        "than the old hand-rolled wider-stencil assembly.");

  auto get_velocity_var = [&](const std::string & param_name)
  {
    return dynamic_cast<const MooseLinearVariableFVReal *>(
        &_fe_problem.getVariable(_tid, getParam<SolverVariableName>(param_name)));
  };

  _velocity_vars[0] = get_velocity_var("u");
  if (!_velocity_vars[0])
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  if (_dim >= 2)
  {
    if (!params.isParamValid("v"))
      paramError("v", "In two or more dimensions, the v velocity must be supplied.");
    _velocity_vars[1] = get_velocity_var("v");
    if (!_velocity_vars[1])
      paramError("v",
                 "In two or more dimensions, the v velocity must be supplied and it must be a "
                 "MooseLinearVariableFVReal.");
  }

  if (_dim >= 3)
  {
    if (!params.isParamValid("w"))
      paramError("w", "In three-dimensions, the w velocity must be supplied.");
    _velocity_vars[2] = get_velocity_var("w");
    if (!_velocity_vars[2])
      paramError("w",
                 "In three-dimensions, the w velocity must be supplied and it must be a "
                 "MooseLinearVariableFVReal.");
  }
}

Real
LinearWCNSFVConservativeMomentumFlux::computeElemMatrixContribution()
{
  return (computeInternalAdvectionElemMatrixContribution() +
          computeInternalStressMatrixContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeNeighborMatrixContribution()
{
  return (computeInternalAdvectionNeighborMatrixContribution() -
          computeInternalStressMatrixContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeElemRightHandSideContribution()
{
  return (computeInternalAdvectionRHSContribution() + computeInternalStressRHSContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeNeighborRightHandSideContribution()
{
  return (-computeInternalAdvectionRHSContribution() - computeInternalStressRHSContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeBoundaryMatrixContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);

  mooseAssert(adv_diff_bc, "This should be a valid BC!");
  return (computeStressBoundaryMatrixContribution(adv_diff_bc) +
          computeAdvectionBoundaryMatrixContribution(adv_diff_bc)) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeBoundaryRHSContribution(
    const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_diff_bc, "This should be a valid BC!");
  return (computeStressBoundaryRHSContribution(adv_diff_bc) +
          computeAdvectionBoundaryRHSContribution(adv_diff_bc)) *
         _current_face_area;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeInternalAdvectionElemMatrixContribution()
{
  return _advected_interp_coeffs.first * _face_mass_flux;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeInternalAdvectionNeighborMatrixContribution()
{
  return _advected_interp_coeffs.second * _face_mass_flux;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeInternalAdvectionRHSContribution()
{
  return _advected_rhs_face_value * _face_mass_flux;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeInternalStressMatrixContribution()
{
  // If we don't have the value yet, we compute it
  if (!_cached_matrix_contribution)
  {
    const auto face_arg = makeCDFace(*_current_face_info);

    // If we requested nonorthogonal correction, we use the normal component of the
    // cell to face vector.
    const auto d = _use_nonorthogonal_correction
                       ? std::abs(_current_face_info->dCN() * _current_face_info->normal())
                       : _current_face_info->dCNMag();

    // Cache the matrix contribution
    _stress_matrix_contribution = _mu(face_arg, determineState()) / d;
    _cached_matrix_contribution = true;
  }

  return _stress_matrix_contribution;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeInternalStressRHSContribution()
{
  // We can have contributions to the right hand side in two occasions:
  // (1) when we use nonorthogonal correction for the normal gradients
  // (2) when we request the deviatoric parts of the stress tensor. (needed for space-dependent
  // viscosities for example)
  if (!_cached_rhs_contribution)
  {
    // scenario (1), we need to add the nonorthogonal correction. In 1D, we don't have
    // any correction so we just skip this part
    if (_dim > 1 && _use_nonorthogonal_correction)
    {
      const auto face_arg = makeCDFace(*_current_face_info);
      const auto state_arg = determineState();

      // Get the gradients from the adjacent cells
      const auto grad_elem = _var.gradSln(*_current_face_info->elemInfo(), state_arg);
      const auto grad_neighbor = _var.gradSln(*_current_face_info->neighborInfo(), state_arg);

      // Interpolate the two gradients to the face
      const auto interp_coeffs =
          interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

      const auto correction_vector =
          _current_face_info->normal() -
          1 / (_current_face_info->normal() * _current_face_info->eCN()) *
              _current_face_info->eCN();

      // Cache the matrix contribution
      _stress_rhs_contribution +=
          _mu(face_arg, state_arg) *
          (interp_coeffs.first * grad_elem + interp_coeffs.second * grad_neighbor) *
          correction_vector;
    }
    // scenario (2), we will have to account for the deviatoric parts of the stress tensor.
    if (_use_deviatoric_terms)
    {
      const auto state_arg = determineState();

      // Interpolate the two gradients to the face
      const auto interp_coeffs =
          interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

      RealGradient grad_elem[3];
      RealGradient grad_neighbor[3];
      Real trace_elem = 0;
      Real trace_neighbor = 0;
      RealVectorValue deviatoric_vector_elem;
      RealVectorValue deviatoric_vector_neighbor;

      // Loop over every velocity component so we can form the symmetric gradient pieces
      for (const auto dir : make_range(_dim))
      {
        grad_elem[dir] = velocityVar(dir).gradSln(*_current_face_info->elemInfo(), state_arg);
        grad_neighbor[dir] =
            velocityVar(dir).gradSln(*_current_face_info->neighborInfo(), state_arg);
        trace_elem += grad_elem[dir](dir);
        trace_neighbor += grad_neighbor[dir](dir);
      }

      const auto face_arg = makeCDFace(*_current_face_info);

      if (_coord_type == Moose::CoordinateSystemType::COORD_RZ)
      {
        Real elem_value = 0.0;
        Real neighbor_value = 0.0;
        const auto & radial_var = velocityVar(_rz_radial_coord);
        elem_value = radial_var.getElemValue(*_current_face_info->elemInfo(), state_arg) /
                     _current_face_info->elemInfo()->centroid()(_rz_radial_coord);
        neighbor_value = radial_var.getElemValue(*_current_face_info->neighborInfo(), state_arg) /
                         _current_face_info->neighborInfo()->centroid()(_rz_radial_coord);

        trace_elem += elem_value;
        trace_neighbor += neighbor_value;
      }

      // Assemble the explicit transpose/trace contribution component by component
      for (const auto dir : make_range(_dim))
      {
        grad_elem[dir](dir) -= 2. / 3 * trace_elem;
        grad_neighbor[dir](dir) -= 2. / 3 * trace_neighbor;

        deviatoric_vector_elem(dir) = grad_elem[dir](_index);
        deviatoric_vector_neighbor(dir) = grad_neighbor[dir](_index);
      }

      _stress_rhs_contribution += _mu(face_arg, state_arg) *
                                  (interp_coeffs.first * deviatoric_vector_elem +
                                   interp_coeffs.second * deviatoric_vector_neighbor) *
                                  _current_face_info->normal();
    }
    _cached_rhs_contribution = true;
  }

  return _stress_rhs_contribution;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeStressBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  auto grad_contrib = bc->computeBoundaryGradientMatrixContribution();
  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!bc->includesMaterialPropertyMultiplier())
  {
    const auto face_arg = singleSidedFaceArg(_current_face_info);
    grad_contrib *= _mu(face_arg, determineState());
  }

  return grad_contrib;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeStressBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto face_arg = singleSidedFaceArg(_current_face_info);
  auto grad_contrib = bc->computeBoundaryGradientRHSContribution();
  // If the boundary condition does not include the diffusivity contribution then
  // add it here.
  if (!bc->includesMaterialPropertyMultiplier())
    grad_contrib *= _mu(face_arg, determineState());

  // We add the nonorthogonal corrector for the face here. Potential idea: we could do
  // this in the boundary condition too. For now, however, we keep it like this.
  if (_use_nonorthogonal_correction && bc->useBoundaryGradientExtrapolation())
  {
    // We support internal boundaries as well. In that case we have to decide on which side
    // of the boundary we are on.
    const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

    // Unit vector to the boundary. Unfortunately, we have to recompute it because the value
    // stored in the face info is only correct for external boundaries
    const auto e_Cf = _current_face_info->faceCentroid() - elem_info->centroid();
    const auto correction_vector =
        _current_face_info->normal() - 1 / (_current_face_info->normal() * e_Cf) * e_Cf;

    const auto state_arg = determineState();
    grad_contrib += _mu(face_arg, state_arg) * _var.gradSln(*elem_info, state_arg) *
                    _boundary_normal_factor * correction_vector;
  }

  if (_use_deviatoric_terms && bc->useBoundaryGradientExtrapolation())
  {
    // We might be on a face which is an internal boundary so we want to make sure we
    // get the gradient from the right side.
    const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

    const auto state_arg = determineState();

    RealGradient grad_elem[3];
    Real trace_elem = 0;
    RealVectorValue deviatoric_vector_elem;

    for (const auto dir : make_range(_dim))
    {
      grad_elem[dir] = velocityVar(dir).gradSln(*elem_info, state_arg);
      trace_elem += grad_elem[dir](dir);
    }

    if (_coord_type == Moose::CoordinateSystemType::COORD_RZ)
    {
      const auto & radial_var = velocityVar(_rz_radial_coord);
      const Real elem_value =
          radial_var.getElemValue(*elem_info, state_arg) / elem_info->centroid()(_rz_radial_coord);
      trace_elem += elem_value;
    }

    for (const auto dir : make_range(_dim))
    {
      grad_elem[dir](dir) -= 2. / 3 * trace_elem;
      deviatoric_vector_elem(dir) = grad_elem[dir](_index);
    }

    // We support internal boundaries too so we have to make sure the normal points always outward
    grad_contrib += _mu(face_arg, state_arg) * deviatoric_vector_elem * _boundary_normal_factor *
                    _current_face_info->normal();
  }

  return grad_contrib;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeAdvectionBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_matrix_contrib = bc->computeBoundaryValueMatrixContribution();
  return boundary_value_matrix_contrib * _face_mass_flux;
}

Real
LinearWCNSFVConservativeMomentumFlux::computeAdvectionBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_rhs_contrib = bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * _face_mass_flux;
}

void
LinearWCNSFVConservativeMomentumFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Multiplier that ensures the normal of the boundary always points outwards, even in cases
  // when the boundary is within the mesh.
  _boundary_normal_factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  // Caching the mass flux on the face which will be reused in the advection term's matrix and
  // right hand side contributions
  if (_mass_flux_functor)
  {
    const auto state = determineState();
    const Moose::FaceArg face_arg = (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
                                        ? makeCDFace(*_current_face_info)
                                        : singleSidedFaceArg(_current_face_info);
    _face_mass_flux = (*_mass_flux_functor)(face_arg, state);
  }
  else
  {
    if (_require_mass_flux_functor)
      mooseError(name(),
                 " cannot assemble momentum advection without mass_flux_functor. The "
                 "sharp-interface conservative path must use the VOF-owned rhoPhi flux.");
    _face_mass_flux = _mass_flux_provider.getMassFlux(*face_info);
  }

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  _advected_rhs_face_value = 0.0;
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH && _adv_interp_method)
  {
    const auto state = determineState();
    const auto & elem_info = *_current_face_info->elemInfo();
    const auto & neighbor_info = *_current_face_info->neighborInfo();
    const Real elem_value = _var.getElemValue(elem_info, state);
    const Real neighbor_value = _var.getElemValue(neighbor_info, state);

    VectorValue<Real> * elem_grad = nullptr;
    VectorValue<Real> * neighbor_grad = nullptr;
    if (_adv_interp_method->needsGradients())
    {
      const auto limiter_type = _adv_interp_method->gradientLimiter();
      _elem_grad_storage = _var.gradSln(elem_info, state, limiter_type);
      _neighbor_grad_storage = _var.gradSln(neighbor_info, state, limiter_type);
      elem_grad = &_elem_grad_storage;
      neighbor_grad = &_neighbor_grad_storage;
    }

    const auto adv_interp = _adv_interp_method->advectedInterpolate(
        *_current_face_info, elem_value, neighbor_value, elem_grad, neighbor_grad, _face_mass_flux);
    _advected_interp_coeffs = adv_interp.weights_matrix;
    _advected_rhs_face_value = adv_interp.rhs_face_value;
  }
  else if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _advected_interp_coeffs =
        interpCoeffs(_advected_interp_method, *_current_face_info, true, _face_mass_flux);
  else
    _advected_interp_coeffs = std::make_pair(0.0, 0.0);

  // We'll have to set this to zero to make sure that we don't accumulate values over multiple
  // faces. The matrix contribution should be fine.
  _stress_rhs_contribution = 0;
}

const MooseLinearVariableFVReal &
LinearWCNSFVConservativeMomentumFlux::velocityVar(unsigned int dir) const
{
  mooseAssert(dir < _velocity_vars.size() && _velocity_vars[dir],
              "Velocity variable for requested direction is not available.");
  return *_velocity_vars[dir];
}
