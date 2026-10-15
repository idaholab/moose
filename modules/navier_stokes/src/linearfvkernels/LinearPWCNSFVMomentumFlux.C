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

  const Real elem_viscosity = _mu(makeElemArg(fi.elemPtr()), state);
  const Real neighbor_viscosity = _mu(makeElemArg(fi.neighborPtr()), state);
  // Like the two-point stress, a side without viscosity transmits no traction.
  if (elem_viscosity == 0.0 || neighbor_viscosity == 0.0)
    return {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  const auto elem_to_face = fi.faceCentroid() - elem_info.centroid();
  const auto face_to_neighbor = neighbor_info.centroid() - fi.faceCentroid();
  const Real elem_distance =
      _use_nonorthogonal_correction ? std::abs(elem_to_face * normal) : elem_to_face.norm();
  const Real neighbor_distance =
      _use_nonorthogonal_correction ? std::abs(face_to_neighbor * normal) : face_to_neighbor.norm();
  const Real elem_porosity = 1.0 / inversePorosity(/*elem_side=*/true);
  const Real neighbor_porosity = 1.0 / inversePorosity(/*elem_side=*/false);

  // The interstitial tangential velocity w_t = U_t / eps is continuous across the baffle and
  // carries a single shear traction q_t. With the face value eps_s w_t,f seen by side s,
  //   q_t = mu_L (eps_L w_t,f - U_t,L) / d_L = mu_R (U_t,R - eps_R w_t,f) / d_R.
  // Eliminating w_t,f gives q_t = T (U_t,R / eps_R - U_t,L / eps_L) with
  //   1 / T = d_L / (mu_L eps_L) + d_R / (mu_R eps_R).
  const Real tangential_transmissibility =
      1.0 / (elem_distance / (elem_viscosity * elem_porosity) +
             neighbor_distance / (neighbor_viscosity * neighbor_porosity));
  // The superficial normal velocity jumps with density, so each side evaluates its normal stress
  //   q_n,s = mu_s (U_n,f,s - U_n,s) / d_s
  // against its own face normal velocity U_n,f,s = mdot_f / rho_s. The inviscid pressure jump
  // carries no normal viscous stress, so these two do not have to balance.
  const Real elem_normal_coefficient = elem_viscosity / elem_distance;
  const Real neighbor_normal_coefficient = neighbor_viscosity / neighbor_distance;
  // The normal stress is treated implicitly with the ordinary two-point stencil and the
  // difference to the one-sided stresses is deferred. Using the half-cell coefficients mu_s / d_s
  // instead would raise the momentum diagonal of baffle-adjacent cells above that of their
  // neighbors, and the face interpolation of 1/A in the Rhie-Chow flux would then disturb the
  // pressure wherever its gradient is nonzero.
  const Real normal_transmissibility = twoPointStressData().transmissibility;

  // The explicit nonorthogonal corrections c_s = grad(U)_s . (n - d_Pf,s / d_s) of each
  // half-cell enter the same balance as the face values.
  RealVectorValue elem_correction;
  RealVectorValue neighbor_correction;
  if (_dim > 1 && _use_nonorthogonal_correction)
  {
    const auto elem_correction_vector = normal - elem_to_face / elem_distance;
    const auto neighbor_correction_vector = normal - face_to_neighbor / neighbor_distance;
    for (const auto component : make_range(_dim))
    {
      const auto & gradient = _porous_mass_flux_provider.velocityGradient(component);
      elem_correction(component) = gradient.gradient(elem_info) * elem_correction_vector;
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

  // Shear traction and side normal stresses with the current velocities. The interstitial
  // corrections follow from eliminating w_t,f with U_t,L - d_L c_t,L and U_t,R + d_R c_t,R in
  // place of the cell values.
  const auto tangential = [&normal](const RealVectorValue & v)
  { return v - (v * normal) * normal; };
  const auto shear_traction =
      tangential_transmissibility *
      tangential((neighbor_velocity + neighbor_distance * neighbor_correction) / neighbor_porosity -
                 (elem_velocity - elem_distance * elem_correction) / elem_porosity);
  const Real elem_normal_stress =
      elem_normal_coefficient *
      (_porous_mass_flux_provider.baffleNormalVelocity(fi, /*elem_side=*/true) -
       elem_velocity * normal + elem_distance * (elem_correction * normal));
  const Real neighbor_normal_stress =
      neighbor_normal_coefficient *
      (neighbor_velocity * normal + neighbor_distance * (neighbor_correction * normal) -
       _porous_mass_flux_provider.baffleNormalVelocity(fi, /*elem_side=*/false));

  // Each side's traction, component _index. The element row adds -traction_L, the neighbor row
  // +traction_R. The parts proportional to the kernel variable are implicit; everything else,
  // including the coupling to the other velocity components, is lagged on the right-hand side.
  const Real n_i = normal(_index);
  const Real tangential_weight = 1.0 - n_i * n_i;
  const Real elem_traction = shear_traction(_index) + elem_normal_stress * n_i;
  const Real neighbor_traction = shear_traction(_index) + neighbor_normal_stress * n_i;

  BaffleStressData data;
  const Real normal_weight = n_i * n_i;
  data.elem_elem = tangential_transmissibility * tangential_weight / elem_porosity +
                   normal_transmissibility * normal_weight;
  data.elem_neighbor = -tangential_transmissibility * tangential_weight / neighbor_porosity -
                       normal_transmissibility * normal_weight;
  data.neighbor_elem = -tangential_transmissibility * tangential_weight / elem_porosity -
                       normal_transmissibility * normal_weight;
  data.neighbor_neighbor = tangential_transmissibility * tangential_weight / neighbor_porosity +
                           normal_transmissibility * normal_weight;
  data.elem_rhs = elem_traction + data.elem_elem * elem_velocity(_index) +
                  data.elem_neighbor * neighbor_velocity(_index);
  data.neighbor_rhs = -neighbor_traction + data.neighbor_elem * elem_velocity(_index) +
                      data.neighbor_neighbor * neighbor_velocity(_index);

  // The deviatoric stress is lagged in each half-cell with that side's viscosity.
  if (_use_deviatoric_terms)
  {
    data.elem_rhs +=
        elem_viscosity * computeCellStressExplicitCorrection(elem_info, RealVectorValue());
    data.neighbor_rhs -=
        neighbor_viscosity * computeCellStressExplicitCorrection(neighbor_info, RealVectorValue());
  }

  return data;
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
