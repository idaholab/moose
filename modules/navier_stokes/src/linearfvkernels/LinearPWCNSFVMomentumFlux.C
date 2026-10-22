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
#include "LinearFVGradientReader.h"
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
      "for the explicit nonorthogonal and deviatoric stress corrections. Required when the "
      "Rhie-Chow object has pressure baffles.");
  return params;
}

LinearPWCNSFVMomentumFlux::LinearPWCNSFVMomentumFlux(const InputParameters & params)
  : LinearWCNSFVMomentumFlux(params),
    _porous_mass_flux_provider(getUserObject<PorousRhieChowMassFlux>("rhie_chow_user_object")),
    _use_two_point_stress_transmissibility(getParam<bool>("use_two_point_stress_transmissibility"))
{
  // The baffle stress splits the two-point flux into one-sided half-cell fluxes, which the
  // interpolated-viscosity flux has no counterpart for.
  if (_porous_mass_flux_provider.hasPressureBaffles() && !_use_two_point_stress_transmissibility)
    paramError("use_two_point_stress_transmissibility",
               "Must be true when PorousRhieChowMassFlux '",
               _porous_mass_flux_provider.name(),
               "' has pressure baffles.");
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

  BaffleStressData stress;
  if (isInternalBaffleFace())
    stress = baffleStressData();
  else
  {
    const Real transmissibility = computeInternalStressMatrixContribution();
    stress = {transmissibility, -transmissibility, -transmissibility, transmissibility, 0.0, 0.0};
  }

  const Real scale_elem = inversePorosity(/*elem_side=*/true);
  const Real scale_neighbor = inversePorosity(/*elem_side=*/false);

  if (hasBlocks(_current_face_info->elemInfo()->subdomain_id()))
  {
    _matrix_contribution(0, 0) = (adv_elem * scale_elem + stress.elem_elem) * _current_face_area;
    _matrix_contribution(0, 1) =
        (adv_neighbor * scale_elem + stress.elem_neighbor) * _current_face_area;
  }

  if (hasBlocks(_current_face_info->neighborInfo()->subdomain_id()))
  {
    _matrix_contribution(1, 0) =
        (-adv_elem * scale_neighbor + stress.neighbor_elem) * _current_face_area;
    _matrix_contribution(1, 1) =
        (-adv_neighbor * scale_neighbor + stress.neighbor_neighbor) * _current_face_area;
  }

  for (auto & matrix : _matrices)
    (*matrix).add_matrix(_matrix_contribution, _dof_indices.get_values());
}

Real
LinearPWCNSFVMomentumFlux::computeElemRightHandSideContribution()
{
  const bool correct_baffle = isInternalBaffleFace();
  const Real stress_rhs =
      (correct_baffle ? baffleStressData().elem_rhs : computeInternalStressRHSContribution()) *
      _current_face_area;
  const Real advection_rhs = correct_baffle
                                 ? 0.0
                                 : _adv_interp_result.rhs_face_value * _face_mass_flux *
                                       inversePorosity(/*elem_side=*/true) * _current_face_area;
  return stress_rhs + advection_rhs + computeBaffleAdvectionExplicitCorrection(/*elem_side=*/true);
}

Real
LinearPWCNSFVMomentumFlux::computeNeighborRightHandSideContribution()
{
  const bool correct_baffle = isInternalBaffleFace();
  const Real stress_rhs =
      (correct_baffle ? baffleStressData().neighbor_rhs : -computeInternalStressRHSContribution()) *
      _current_face_area;
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
  if (!isInternalBaffleFace())
    return 0.0;

  const auto state_arg = determineState();

  const auto & fi = *_current_face_info;
  const Real u_elem = _var.getElemValue(*fi.elemInfo(), state_arg);
  const Real u_neighbor = _var.getElemValue(*fi.neighborInfo(), state_arg);
  // Keep the shared internal-face stencil, then explicitly replace the interpolated state on
  // baffle faces. Interpolating across the baffle would mix the two sides of the velocity jump,
  // and the cell value alone would miss the variation between the cell center and the face.
  const Real shared_advected_state = _adv_interp_result.weights_matrix.first * u_elem +
                                     _adv_interp_result.weights_matrix.second * u_neighbor;

  // With V_s the velocity of side s extrapolated to the face, both sides advect the interstitial
  // tangential velocity u_t,f = (I - n n) V_up / eps_up of the upstream side, so the tangential
  // momentum flux mdot u_t,f leaving one side enters the other and the face is no source of
  // tangential momentum. Each side keeps its own normal velocity V_s . n, and the pressure jump
  // balances the normal momentum difference. Side s therefore advects
  //   V*_s = eps_s u_t,f + (V_s . n) n.
  const auto & normal = fi.normal();
  const bool upstream_elem_side = _face_mass_flux >= 0.0;
  const auto upstream_velocity =
      _porous_mass_flux_provider.baffleFaceVelocity(fi, upstream_elem_side);
  const auto interstitial_tangential_velocity =
      (upstream_velocity - (upstream_velocity * normal) * normal) *
      inversePorosity(upstream_elem_side);
  const Real own_normal_velocity =
      _porous_mass_flux_provider.baffleFaceVelocity(fi, elem_side) * normal;
  const Real baffle_advected_state =
      interstitial_tangential_velocity(_index) / inversePorosity(elem_side) +
      own_normal_velocity * normal(_index);
  const Real factor = elem_side ? 1.0 : -1.0;

  return factor * _face_mass_flux * inversePorosity(elem_side) *
         (shared_advected_state - baffle_advected_state) * _current_face_area;
}

bool
LinearPWCNSFVMomentumFlux::isInternalBaffleFace() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::BOTH &&
         _porous_mass_flux_provider.faceIsBaffle(*_current_face_info);
}

LinearPWCNSFVMomentumFlux::BaffleStressData
LinearPWCNSFVMomentumFlux::baffleStressData() const
{
  const auto & fi = *_current_face_info;
  const auto & elem_info = *fi.elemInfo();
  const auto & neighbor_info = *fi.neighborInfo();
  const auto state = determineState();
  const auto & normal = fi.normal();

  // A side of unit porosity is clear fluid; any other side is a porous medium. The viscous
  // (Brinkman) layer on the porous side of an interface is of the order of the square root of the
  // permeability, much thinner than the cells next to a lumped interface. Clear fluid therefore
  // shears against the porous side as against a rough wall moving with the superficial velocity
  // of the porous cell, and two porous sides transmit no shear.
  const bool elem_clear = isClearFluidSide(/*elem_side=*/true);
  const bool neighbor_clear = isClearFluidSide(/*elem_side=*/false);
  if (!elem_clear && !neighbor_clear)
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  const Real elem_viscosity = _mu(makeElemArg(fi.elemPtr()), state);
  const Real neighbor_viscosity = _mu(makeElemArg(fi.neighborPtr()), state);
  // Like the two-point stress, an inviscid clear side transmits no traction.
  if ((elem_clear && elem_viscosity == 0.0) || (neighbor_clear && neighbor_viscosity == 0.0))
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  const auto elem_to_face = fi.faceCentroid() - elem_info.centroid();
  const auto face_to_neighbor = neighbor_info.centroid() - fi.faceCentroid();
  const Real elem_distance =
      _use_nonorthogonal_correction ? std::abs(elem_to_face * normal) : elem_to_face.norm();
  const Real neighbor_distance =
      _use_nonorthogonal_correction ? std::abs(face_to_neighbor * normal) : face_to_neighbor.norm();

  // The shear traction q_t = T (I - n n) (U_N,f - U_P,f) acts between the superficial face
  // velocities of the two sides. A clear side extrapolates its face velocity across its half-cell,
  // whose resistance is d_s / mu_s; a porous side's face velocity is its cell value (we impose this, and thus there is no resistance on the porous side). Hence
  //   1 / T = sum of d_s / mu_s over the clear sides,
  // which for two clear sides is the harmonic two-point transmissibility.
  Real resistance = 0.0;
  if (elem_clear)
    resistance += elem_distance / elem_viscosity;
  if (neighbor_clear)
    resistance += neighbor_distance / neighbor_viscosity;
  const Real transmissibility = 1.0 / resistance;

  // The explicit nonorthogonal corrections c_s = grad(U)_s . (n - d_Pf,s / d_s) of each clear
  // half-cell enter the same balance as the face values.
  // No stress contribution on the porous side thus no correction.
  RealVectorValue elem_correction;
  RealVectorValue neighbor_correction;
  if (_dim > 1 && _use_nonorthogonal_correction)
  {
    const auto elem_correction_vector = normal - elem_to_face / elem_distance;
    const auto neighbor_correction_vector = normal - face_to_neighbor / neighbor_distance;
    for (const auto component : make_range(_dim))
    {
      const auto & gradient = _porous_mass_flux_provider.velocityGradient(component);
      if (elem_clear)
        elem_correction(component) = gradient.gradient(elem_info) * elem_correction_vector;
      if (neighbor_clear)
        neighbor_correction(component) =
            gradient.gradient(neighbor_info) * neighbor_correction_vector;
    }
  }

  RealVectorValue elem_velocity;
  RealVectorValue neighbor_velocity;
  for (const auto component : make_range(_dim))
  {
    const auto & velocity = velocityVar(component);
    elem_velocity(component) = velocity.getElemValue(elem_info, state);
    neighbor_velocity(component) = velocity.getElemValue(neighbor_info, state);
  }

  const auto face_velocity_difference =
      (neighbor_velocity + neighbor_distance * neighbor_correction) -
      (elem_velocity - elem_distance * elem_correction);
  const auto shear_traction =
      transmissibility * (face_velocity_difference - (face_velocity_difference * normal) * normal);

  // There is no normal viscous stress on a baffle face: the pressure jump carries the normal
  // momentum balance, and a two-point normal stress would read the jump of mdot / rho_s as a
  // gradient. The element row adds -q_t, the neighbor row +q_t. The part proportional to the
  // kernel variable is implicit; everything else, including the coupling to the other velocity
  // components, is lagged on the right-hand side.
  const Real n_i = normal(_index);
  const Real coefficient = transmissibility * (1.0 - n_i * n_i);

  BaffleStressData data;
  data.elem_elem = coefficient;
  data.elem_neighbor = -coefficient;
  data.neighbor_elem = -coefficient;
  data.neighbor_neighbor = coefficient;
  data.elem_rhs =
      shear_traction(_index) + coefficient * (elem_velocity(_index) - neighbor_velocity(_index));
  data.neighbor_rhs =
      -shear_traction(_index) + coefficient * (neighbor_velocity(_index) - elem_velocity(_index));

  // The deviatoric stress is lagged in each clear half-cell with that side's viscosity.
  if (_use_deviatoric_terms)
  {
    if (elem_clear)
      data.elem_rhs +=
          elem_viscosity * computeCellStressExplicitCorrection(elem_info, RealVectorValue());
    if (neighbor_clear)
      data.neighbor_rhs -= neighbor_viscosity *
                           computeCellStressExplicitCorrection(neighbor_info, RealVectorValue());
  }

  return data;
}

bool
LinearPWCNSFVMomentumFlux::isClearFluidSide(const bool elem_side) const
{
  return MooseUtils::absoluteFuzzyEqual(1.0 / inversePorosity(elem_side), 1.0);
}

Real
LinearPWCNSFVMomentumFlux::inversePorosity(const bool elem_side) const
{
  const Real porosity = _porous_mass_flux_provider.getFaceSidePorosity(
      *_current_face_info, elem_side, determineState());
  if (porosity <= 0.0)
    mooseError("porosity must be positive on face ", _current_face_info->id(), ".");

  return 1.0 / porosity;
}
