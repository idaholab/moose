//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearPWCNSFVMomentumFlux.h"
#include "LinearFVAdvectionDiffusionBC.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesApp", LinearPWCNSFVMomentumFlux);

InputParameters
LinearPWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearWCNSFVMomentumFlux::validParams();
  params.addClassDescription("Momentum flux kernel with porous-specific advection handling.");
  params.addParam<bool>(
      "use_two_point_stress_transmissibility",
      false,
      "Use two-point harmonic transmissibility for the stress term instead of a face-interpolated "
      "gradient.");
  return params;
}

LinearPWCNSFVMomentumFlux::LinearPWCNSFVMomentumFlux(const InputParameters & params)
  : LinearWCNSFVMomentumFlux(params),
    _use_two_point_stress_transmissibility(getParam<bool>("use_two_point_stress_transmissibility"))
{
}

Real
LinearPWCNSFVMomentumFlux::computeInternalStressTransmissibility() const
{
  if (!_use_two_point_stress_transmissibility)
    return LinearWCNSFVMomentumFlux::computeInternalStressTransmissibility();

  const auto & fi = *_current_face_info;
  const auto state = determineState();
  const Real elem_viscosity = _mu(makeElemArg(fi.elemPtr()), state);
  const Real neighbor_viscosity = _mu(makeElemArg(fi.neighborPtr()), state);
  if (elem_viscosity == 0.0 || neighbor_viscosity == 0.0)
    return 0.0;

  const Real elem_distance = (fi.faceCentroid() - fi.elemCentroid()).norm();
  const Real neighbor_distance = (fi.neighborCentroid() - fi.faceCentroid()).norm();
  if (elem_distance == 0.0 || neighbor_distance == 0.0)
    return 0.0;

  return 1.0 / (elem_distance / elem_viscosity + neighbor_distance / neighbor_viscosity);
}

Real
LinearPWCNSFVMomentumFlux::computeInternalStressExplicitCorrection() const
{
  return _use_two_point_stress_transmissibility
             ? 0.0
             : LinearWCNSFVMomentumFlux::computeInternalStressExplicitCorrection();
}

void
LinearPWCNSFVMomentumFlux::addMatrixContribution()
{
  if (_current_face_type != FaceInfo::VarFaceNeighbors::BOTH)
  {
    LinearFVFluxKernel::addMatrixContribution();
    return;
  }

  _dof_indices(0) = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
  _dof_indices(1) = _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];

  const Real adv_elem = computeInternalAdvectionElemMatrixContribution();
  const Real adv_neighbor = computeInternalAdvectionNeighborMatrixContribution();
  const Real stress = computeInternalStressMatrixContribution();

  const Real scale_elem = inversePorosity(/*elem_side=*/true);
  const Real scale_neighbor = inversePorosity(/*elem_side=*/false);

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
LinearPWCNSFVMomentumFlux::computeElemMatrixContribution()
{
  const Real stress = computeInternalStressMatrixContribution();
  return (computeInternalAdvectionElemMatrixContribution() * inversePorosity(/*elem_side=*/true) +
          stress) *
         _current_face_area;
}

Real
LinearPWCNSFVMomentumFlux::computeNeighborMatrixContribution()
{
  const Real stress = computeInternalStressMatrixContribution();
  return (computeInternalAdvectionNeighborMatrixContribution() *
              inversePorosity(/*elem_side=*/false) -
          stress) *
         _current_face_area;
}

Real
LinearPWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  const bool correct_baffle = needsInternalBaffleAdvectionCorrection();
  const Real stress_rhs = computeInternalStressRHSContribution() * _current_face_area;
  const Real advection_rhs = correct_baffle
                                 ? 0.0
                                 : _adv_interp_result.rhs_face_value * _face_mass_flux *
                                       inversePorosity(/*elem_side=*/true) * _current_face_area;
  return stress_rhs + advection_rhs + computeBaffleAdvectionExplicitCorrection(/*elem_side=*/true);
}

Real
LinearPWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  const bool correct_baffle = needsInternalBaffleAdvectionCorrection();
  const Real stress_rhs = -computeInternalStressRHSContribution() * _current_face_area;
  const Real advection_rhs = correct_baffle
                                 ? 0.0
                                 : -_adv_interp_result.rhs_face_value * _face_mass_flux *
                                       inversePorosity(/*elem_side=*/false) * _current_face_area;
  return stress_rhs + advection_rhs + computeBaffleAdvectionExplicitCorrection(/*elem_side=*/false);
}

Real
LinearPWCNSFVMomentumFlux::computeAdvectionBoundaryMatrixContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_matrix_contrib = bc->computeBoundaryValueMatrixContribution();
  const bool elem_side = _current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR;
  return boundary_value_matrix_contrib * _face_mass_flux * inversePorosity(elem_side);
}

Real
LinearPWCNSFVMomentumFlux::computeAdvectionBoundaryRHSContribution(
    const LinearFVAdvectionDiffusionBC * bc)
{
  const auto boundary_value_rhs_contrib = bc->computeBoundaryValueRHSContribution();
  const bool elem_side = _current_face_type != FaceInfo::VarFaceNeighbors::NEIGHBOR;
  return -boundary_value_rhs_contrib * _face_mass_flux * inversePorosity(elem_side);
}

Real
LinearPWCNSFVMomentumFlux::computeBaffleAdvectionExplicitCorrection(bool elem_side) const
{
  if (!needsInternalBaffleAdvectionCorrection())
    return 0.0;

  const auto state_arg = determineState();

  const Real u_elem = _var.getElemValue(*_current_face_info->elemInfo(), state_arg);
  const Real u_neighbor = _var.getElemValue(*_current_face_info->neighborInfo(), state_arg);
  // Keep the shared internal-face stencil, then explicitly cancel the interpolated part on
  // one-sided baffle faces so each side advects with its local state only.
  const Real shared_advected_state = _adv_interp_result.weights_matrix.first * u_elem +
                                     _adv_interp_result.weights_matrix.second * u_neighbor;
  const Real one_sided_advected_state = elem_side ? u_elem : u_neighbor;
  const Real factor = elem_side ? 1.0 : -1.0;

  return factor * _face_mass_flux * inversePorosity(elem_side) *
         (shared_advected_state - one_sided_advected_state) * _current_face_area;
}

bool
LinearPWCNSFVMomentumFlux::isInternalBaffleFace() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::BOTH &&
         _mass_flux_provider.faceIsBaffle(*_current_face_info);
}

bool
LinearPWCNSFVMomentumFlux::needsInternalBaffleAdvectionCorrection() const
{
  return isInternalBaffleFace() &&
         _mass_flux_provider.faceUsesOneSidedReconstruction(*_current_face_info);
}

Real
LinearPWCNSFVMomentumFlux::inversePorosity(const bool elem_side) const
{
  const Real porosity =
      _mass_flux_provider.getFaceSidePorosity(*_current_face_info, elem_side, determineState());
  if (porosity <= 0.0)
    mooseError(name(), ": porosity must be positive on face ", _current_face_info->id(), ".");

  return 1.0 / porosity;
}
