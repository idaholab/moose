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
#include "MathFVUtils.h"
#include "PorousRhieChowMassFlux.h"

#include <cmath>

registerMooseObject("NavierStokesApp", LinearPWCNSFVMomentumFlux);

InputParameters
LinearPWCNSFVMomentumFlux::validParams()
{
  InputParameters params = LinearWCNSFVMomentumFlux::validParams();
  params.addClassDescription("Momentum flux kernel with porous-specific advection handling.");
  params.addParam<bool>(
      "use_two_point_stress_transmissibility",
      false,
      "Use a harmonic two-point treatment for the normal stress term and side-local gradients "
      "for the explicit nonorthogonal and deviatoric stress corrections.");
  return params;
}

LinearPWCNSFVMomentumFlux::LinearPWCNSFVMomentumFlux(const InputParameters & params)
  : LinearWCNSFVMomentumFlux(params),
    _porous_mass_flux_provider(getUserObject<PorousRhieChowMassFlux>("rhie_chow_user_object")),
    _use_two_point_stress_transmissibility(getParam<bool>("use_two_point_stress_transmissibility"))
{
}

Real
LinearPWCNSFVMomentumFlux::computeInternalStressTransmissibility() const
{
  if (!_use_two_point_stress_transmissibility)
    return LinearWCNSFVMomentumFlux::computeInternalStressTransmissibility();

  return twoPointStressData().transmissibility;
}

LinearPWCNSFVMomentumFlux::TwoPointStressData
LinearPWCNSFVMomentumFlux::twoPointStressData() const
{
  const auto & fi = *_current_face_info;
  const auto state = determineState();
  const Real elem_viscosity = _mu(makeElemArg(fi.elemPtr()), state);
  const Real neighbor_viscosity = _mu(makeElemArg(fi.neighborPtr()), state);
  if (elem_viscosity == 0.0 || neighbor_viscosity == 0.0)
    return {0.0, 0.0, 0.0};

  const auto elem_to_face = fi.faceCentroid() - fi.elemCentroid();
  const auto face_to_neighbor = fi.neighborCentroid() - fi.faceCentroid();
  const Real elem_distance =
      _use_nonorthogonal_correction ? std::abs(elem_to_face * fi.normal()) : elem_to_face.norm();
  const Real neighbor_distance = _use_nonorthogonal_correction
                                     ? std::abs(face_to_neighbor * fi.normal())
                                     : face_to_neighbor.norm();
  if (elem_distance == 0.0 || neighbor_distance == 0.0)
    return {0.0, elem_distance, neighbor_distance};

  const Real total_distance = elem_distance + neighbor_distance;
  const Real face_viscosity = Moose::FV::harmonicInterpolation(elem_viscosity,
                                                               neighbor_viscosity,
                                                               elem_distance / total_distance,
                                                               neighbor_distance / total_distance);
  return {face_viscosity / total_distance, elem_distance, neighbor_distance};
}

Real
LinearPWCNSFVMomentumFlux::computeInternalStressExplicitCorrection() const
{
  if (!_use_two_point_stress_transmissibility)
    return LinearWCNSFVMomentumFlux::computeInternalStressExplicitCorrection();

  if ((!_use_nonorthogonal_correction || _dim == 1) && !_use_deviatoric_terms)
    return 0.0;

  const auto & fi = *_current_face_info;
  const auto stress_data = twoPointStressData();
  if (stress_data.transmissibility == 0.0)
    return 0.0;

  RealVectorValue elem_correction_vector;
  RealVectorValue neighbor_correction_vector;

  if (_dim > 1 && _use_nonorthogonal_correction)
  {
    const auto elem_to_face = fi.faceCentroid() - fi.elemCentroid();
    const auto face_to_neighbor = fi.neighborCentroid() - fi.faceCentroid();

    // Apply a separate tangential reconstruction over each half-cell, then combine both
    // corrections through the same series resistance as the normal two-point flux. This recovers
    // the exact normal gradient for a linear field without interpolating a gradient across a
    // material jump.
    elem_correction_vector = fi.normal() - elem_to_face / stress_data.elem_distance;
    neighbor_correction_vector = fi.normal() - face_to_neighbor / stress_data.neighbor_distance;
  }

  const Real elem_correction =
      computeCellStressExplicitCorrection(*fi.elemInfo(), elem_correction_vector);
  const Real neighbor_correction =
      computeCellStressExplicitCorrection(*fi.neighborInfo(), neighbor_correction_vector);

  // Eliminate the common face value while enforcing one total traction on both half-cells. Both
  // explicit corrections must participate in the same series-resistance balance as the normal
  // two-point term; averaging them separately would not preserve an exact traction across a
  // viscosity jump.
  return stress_data.transmissibility * (stress_data.elem_distance * elem_correction +
                                         stress_data.neighbor_distance * neighbor_correction);
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
         _porous_mass_flux_provider.faceIsBaffle(*_current_face_info);
}

bool
LinearPWCNSFVMomentumFlux::needsInternalBaffleAdvectionCorrection() const
{
  return isInternalBaffleFace() &&
         _porous_mass_flux_provider.faceUsesOneSidedReconstruction(*_current_face_info);
}

Real
LinearPWCNSFVMomentumFlux::inversePorosity(const bool elem_side) const
{
  const Real porosity = _porous_mass_flux_provider.getFaceSidePorosity(
      *_current_face_info, elem_side, determineState());
  if (porosity <= 0.0)
    mooseError(name(), ": porosity must be positive on face ", _current_face_info->id(), ".");

  return 1.0 / porosity;
}
