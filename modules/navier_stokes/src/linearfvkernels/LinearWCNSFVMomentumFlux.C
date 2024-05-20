//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFVMomentumFlux.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearWCNSFVMomentumFlux);

InputParameters
LinearWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearFVFluxKernel::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of the "
                             "stress and advection terms of the momentum equation.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which is used to determine the face velocity.");
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
  return params;
}

LinearWCNSFVMomentumFlux::LinearWCNSFVMomentumFlux(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _mu(getFunctor<Real>(getParam<MooseFunctorName>(NS::mu))),
    _use_nonorthogonal_correction(getParam<bool>("use_nonorthogonal_correction")),
    _use_deviatoric_terms(getParam<bool>("use_deviatoric_terms")),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _face_mass_flux(0.0),
    _stress_matrix_contribution(0.0),
    _stress_rhs_contribution(0.0),
    _index(getParam<MooseEnum>("momentum_component")),
    _u_var(dynamic_cast<const MooseLinearVariableFVReal *>(
        &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("u")))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("v")))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("w")))
               : nullptr)
{
  // We only need gradients if the nonorthogonal correction is enabled or when we request the
  // computation of the deviatoric parts of the stress tensor.
  if (_use_nonorthogonal_correction || _use_deviatoric_terms)
    _var.computeCellGradients();

  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");

  if (!_u_var)
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
}

Real
LinearWCNSFVMomentumFlux::computeElemMatrixContribution()
{
  return (computeInternalAdvectionElemMatrixContribution() +
          computeInternalStressMatrixContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborMatrixContribution()
{
  return (computeInternalAdvectionNeighborMatrixContribution() -
          computeInternalStressMatrixContribution()) *
         _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  return computeInternalStressRHSContribution() * _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  return -computeInternalStressRHSContribution() * _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_diff_bc, "This should be a valid BC!");
  return (computeStressBoundaryMatrixContribution(adv_diff_bc) +
          computeAdvectionBoundaryMatrixContribution(adv_diff_bc)) *
         _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc)
{
  const auto * const adv_diff_bc = static_cast<const LinearFVAdvectionDiffusionBC *>(&bc);
  mooseAssert(adv_diff_bc, "This should be a valid BC!");
  return (computeStressBoundaryRHSContribution(adv_diff_bc) +
          computeAdvectionBoundaryRHSContribution(adv_diff_bc)) *
         _current_face_area;
}

Real
LinearWCNSFVMomentumFlux::computeInternalAdvectionElemMatrixContribution()
{
  return _advected_interp_coeffs.first * _face_mass_flux;
}

Real
LinearWCNSFVMomentumFlux::computeInternalAdvectionNeighborMatrixContribution()
{
  return _advected_interp_coeffs.second * _face_mass_flux;
}

Real
LinearWCNSFVMomentumFlux::computeInternalStressMatrixContribution()
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
LinearWCNSFVMomentumFlux::computeInternalStressRHSContribution()
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
      const auto grad_elem = _var.gradSln(*_current_face_info->elemInfo());
      const auto & grad_neighbor = _var.gradSln(*_current_face_info->neighborInfo());

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
    else if (_use_deviatoric_terms)
    {
      // Interpolate the two gradients to the face
      const auto interp_coeffs =
          interpCoeffs(Moose::FV::InterpMethod::Average, *_current_face_info, true);

      const auto u_grad_elem = _u_var->gradSln(*_current_face_info->elemInfo());
      const auto u_grad_neighbor = _u_var->gradSln(*_current_face_info->neighborInfo());

      Real trace_elem = 0;
      Real trace_neighbor = 0;
      RealVectorValue deviatoric_vector_elem;
      RealVectorValue deviatoric_vector_neighbor;

      deviatoric_vector_elem(0) = u_grad_elem(_index);
      deviatoric_vector_neighbor(0) = u_grad_neighbor(_index);
      trace_elem += u_grad_elem(0);
      trace_neighbor += u_grad_neighbor(0);

      const auto face_arg = makeCDFace(*_current_face_info);
      const auto state_arg = determineState();

      if (_dim > 1)
      {
        const auto v_grad_elem = _v_var->gradSln(*_current_face_info->elemInfo());
        const auto v_grad_neighbor = _v_var->gradSln(*_current_face_info->neighborInfo());

        deviatoric_vector_elem(1) = v_grad_elem(_index);
        deviatoric_vector_neighbor(1) = v_grad_neighbor(_index);
        trace_elem += v_grad_elem(1);
        trace_neighbor += v_grad_neighbor(1);
        if (_dim > 2)
        {
          const auto w_grad_elem = _w_var->gradSln(*_current_face_info->elemInfo());
          const auto w_grad_neighbor = _w_var->gradSln(*_current_face_info->neighborInfo());

          deviatoric_vector_elem(2) = w_grad_elem(_index);
          deviatoric_vector_neighbor(2) = w_grad_neighbor(_index);
          trace_elem += w_grad_elem(2);
          trace_neighbor += w_grad_neighbor(2);
        }
      }
      deviatoric_vector_elem(_index) = trace_elem;
      deviatoric_vector_neighbor(_index) = trace_neighbor;

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
LinearWCNSFVMomentumFlux::computeStressBoundaryMatrixContribution(
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
LinearWCNSFVMomentumFlux::computeStressBoundaryRHSContribution(
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
  if (_use_nonorthogonal_correction)
  {
    const auto correction_vector =
        _current_face_info->normal() -
        1 / (_current_face_info->normal() * _current_face_info->eCN()) * _current_face_info->eCN();

    grad_contrib += _mu(face_arg, determineState()) *
                    _var.gradSln(*_current_face_info->elemInfo()) * correction_vector;
  }

  return grad_contrib;
}

Real
LinearWCNSFVMomentumFlux::computeAdvectionBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_matrix_contrib = bc->computeBoundaryValueMatrixContribution();

  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return boundary_value_matrix_contrib * factor * _face_mass_flux;
}

Real
LinearWCNSFVMomentumFlux::computeAdvectionBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  // We support internal boundaries too so we have to make sure the normal points always outward
  const auto factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ? 1.0 : -1.0);

  const auto boundary_value_rhs_contrib = bc->computeBoundaryValueRHSContribution();
  return -boundary_value_rhs_contrib * factor * _face_mass_flux;
}

void
LinearWCNSFVMomentumFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);

  // Caching the mass flux on the face which will be reused in the advection term's matrix and right
  // hand side contributions
  _face_mass_flux = _mass_flux_provider.getMassFlux(*face_info);

  // Caching the interpolation coefficients so they will be reused for the matrix and right hand
  // side terms
  _advected_interp_coeffs =
      interpCoeffs(_advected_interp_method, *_current_face_info, true, _face_mass_flux);

  // We'll have to set this to zero to make sure that we don't accumulate values over multiple
  // faces. The matrix contribution should be fine.
  _stress_rhs_contribution = 0;
}
