//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousLinearWCNSFVMomentumFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", PorousLinearWCNSFVMomentumFlux);

InputParameters
PorousLinearWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearWCNSFVMomentumFlux::validParams();
  params.addClassDescription("Momentum flux kernel with porous-specific advection handling.");
  // params.set<bool>("use_two_point_stress_transmissibility") = false;
  params.addParam<bool>("porosity_outside_divergence",
                        false,
                        "Scale the advection term by 1/porosity outside the divergence operator "
                        "(i.e. do not scale the advected interpolation by 1/eps).");
  return params;
}

PorousLinearWCNSFVMomentumFlux::PorousLinearWCNSFVMomentumFlux(const InputParameters & params)
  : LinearWCNSFVMomentumFlux(params),
    _porosity_outside_divergence(getParam<bool>("porosity_outside_divergence"))
{
  _var.computeCellGradients();
}

void
PorousLinearWCNSFVMomentumFlux::setupFaceData(const FaceInfo * face_info)
{
  LinearWCNSFVMomentumFlux::setupFaceData(face_info);

  _advected_interp_coeffs = _mass_flux_provider.getAdvectedInterpolationCoeffs(
      *_current_face_info,
      _advected_interp_method,
      _face_mass_flux,
      /*apply_porosity_scaling=*/!_porosity_outside_divergence);
}

void
PorousLinearWCNSFVMomentumFlux::addMatrixContribution()
{
  if (!_porosity_outside_divergence || _current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
  {
    LinearFVFluxKernel::addMatrixContribution();
    return;
  }

  _dof_indices(0) = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
  _dof_indices(1) = _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];

  const Real adv_elem = computeInternalAdvectionElemMatrixContribution();
  const Real adv_neighbor = computeInternalAdvectionNeighborMatrixContribution();
  const Real stress = computeInternalStressMatrixContribution();

  const auto time_arg = determineState();
  const Real eps_elem =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, /*elem_side=*/true, time_arg);
  const Real eps_neighbor =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, /*elem_side=*/false, time_arg);
  const Real scale_elem = eps_elem != 0.0 ? 1.0 / eps_elem : 0.0;
  const Real scale_neighbor = eps_neighbor != 0.0 ? 1.0 / eps_neighbor : 0.0;

  if (hasBlocks(_current_face_info->elemInfo()->subdomain_id()))
  {
    _matrix_contribution(0, 0) = (adv_elem * scale_elem + stress) * _current_face_area;
    _matrix_contribution(0, 1) = (adv_neighbor * scale_elem - stress) * _current_face_area;
  }

  if (hasBlocks(_current_face_info->neighborInfo()->subdomain_id()))
  {
    _matrix_contribution(1, 0) = (-adv_elem * scale_neighbor - stress) * _current_face_area;
    _matrix_contribution(1, 1) = (-adv_neighbor * scale_neighbor + stress) * _current_face_area;
  }

  for (auto & matrix : _matrices)
    (*matrix).add_matrix(_matrix_contribution, _dof_indices.get_values());
}

Real
PorousLinearWCNSFVMomentumFlux::computeElemMatrixContribution()
{
  const Real stress = computeInternalStressMatrixContribution();
  return (computeInternalAdvectionElemMatrixContribution() + stress) * _current_face_area;
}

Real
PorousLinearWCNSFVMomentumFlux::computeNeighborMatrixContribution()
{
  const Real stress = computeInternalStressMatrixContribution();
  return (computeInternalAdvectionNeighborMatrixContribution() - stress) * _current_face_area;
}

Real
PorousLinearWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  const Real stress_rhs = LinearWCNSFVMomentumFlux::computeElemRightHandSideContribution();
  return stress_rhs +
         computeBaffleAdvectionExplicitCorrection(/*elem_side=*/true);
}

Real
PorousLinearWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  const Real stress_rhs = LinearWCNSFVMomentumFlux::computeNeighborRightHandSideContribution();
  return stress_rhs +
         computeBaffleAdvectionExplicitCorrection(/*elem_side=*/false);
}

Real
PorousLinearWCNSFVMomentumFlux::computeAdvectionBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_matrix_contrib = bc->computeBoundaryValueMatrixContribution();
  const bool elem_side = (_current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR);
  const Real eps =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, elem_side, determineState());
  const Real scale = eps != 0.0 ? 1.0 / eps : 0.0;
  return boundary_value_matrix_contrib * _face_mass_flux * scale;
}

Real
PorousLinearWCNSFVMomentumFlux::computeAdvectionBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_rhs_contrib = bc->computeBoundaryValueRHSContribution();
  const bool elem_side = (_current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR);
  const Real eps =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, elem_side, determineState());
  const Real scale = eps != 0.0 ? 1.0 / eps : 0.0;
  return -boundary_value_rhs_contrib * _face_mass_flux * scale;
}

Real
PorousLinearWCNSFVMomentumFlux::computeBaffleAdvectionExplicitCorrection(bool elem_side) const
{
  if (!_porosity_outside_divergence || !needsInternalBaffleAdvectionCorrection())
    return 0.0;

  const auto state_arg = determineState();
  const Real eps =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, elem_side, state_arg);
  if (eps == 0.0)
    return 0.0;

  const Real u_elem = _var.getElemValue(*_current_face_info->elemInfo(), state_arg);
  const Real u_neighbor = _var.getElemValue(*_current_face_info->neighborInfo(), state_arg);
  // Keep the shared internal-face stencil, then explicitly cancel the interpolated part on
  // one-sided baffle faces so each side advects with its local state only.
  const Real shared_advected_state =
      _advected_interp_coeffs.first * u_elem + _advected_interp_coeffs.second * u_neighbor;
  const Real one_sided_advected_state = elem_side ? u_elem : u_neighbor;
  const Real factor = elem_side ? 1.0 : -1.0;

  return factor * _face_mass_flux / eps * (shared_advected_state - one_sided_advected_state) *
         _current_face_area;
}

bool
PorousLinearWCNSFVMomentumFlux::isInternalBaffleFace() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::BOTH &&
         _mass_flux_provider.faceIsBaffle(*_current_face_info);
}

bool
PorousLinearWCNSFVMomentumFlux::needsInternalBaffleAdvectionCorrection() const
{
  return isInternalBaffleFace() &&
         _mass_flux_provider.faceUsesOneSidedReconstruction(*_current_face_info);
}
