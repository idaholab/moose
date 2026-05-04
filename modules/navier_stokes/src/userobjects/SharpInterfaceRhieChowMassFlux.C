#include "SharpInterfaceRhieChowMassFlux.h"

#include "LinearFVPressureInletOutletVelocityBC.h"
#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "PIMPLE.h"
#include "ReducedPressurePIMPLE.h"
#include "SIMPLE.h"
#include "SubProblem.h"
#include "FVUtils.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

registerMooseObject("NavierStokesApp", SharpInterfaceRhieChowMassFlux);

InputParameters
SharpInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();

  params.addClassDescription(
      "Rhie-Chow face-flux provider with additional reduced-pressure predictor functors for "
      "large-density-ratio sharp-interface coupling.");

  params.addParam<bool>(
      "add_transient_projection_flux",
      true,
      "Whether to publish a pressure-equation source flux corresponding to a transient "
      "projection correction.");
  params.addParam<bool>(
      "add_capillary_hydrostatic_flux",
      true,
      "Whether to publish a pressure-equation source flux corresponding to explicit "
      "capillary and hydrostatic-density-gradient corrections.");
  params.addParam<bool>(
      "use_face_based_predictor_body_force",
      true,
      "Whether to reconstruct the reduced-pressure momentum-predictor explicit forcing from "
      "face-based sharp-interface operators instead of the legacy cell-gradient / cell-"
      "acceleration path.");

  params.addParam<MooseFunctorName>(
      "transient_projection_face_acceleration",
      "",
      "Optional face-vector functor containing the already-discretized transient projection "
      "correction in acceleration-like units. The published pressure-equation source flux will "
      "be built as -(rho_f * (Ainv_raw,f o accel_f) . n_f).");
  params.addParam<MooseFunctorName>(
      "surface_tension_face_acceleration",
      "",
      "Optional face-vector functor containing the surface-tension contribution in the "
      "reduced-pressure momentum balance, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "surface_tension_cell_acceleration",
      "",
      "Optional cell-vector functor containing the surface-tension contribution in the "
      "reduced-pressure momentum balance, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_face_acceleration",
      "",
      "Optional face-vector functor containing the reduced-pressure hydrostatic density-gradient "
      "contribution, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_cell_acceleration",
      "",
      "Optional cell-vector functor containing the reduced-pressure hydrostatic density-gradient "
      "contribution, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "vof_rho_phi_functor",
      "rho_phi",
      "Optional alpha-consistent density flux published by the sharp-interface VOF transport. "
      "When present, downstream advection queries use this face mass flux instead of the raw "
      "Rhie-Chow density interpolation.");
  params.addParam<MooseFunctorName>(
      "vof_alpha_phi_limited_functor",
      "alpha_phi_limited",
      "Optional limited alpha face flux published by the sharp-interface VOF transport. When "
      "provided together with vof_rho_phi_functor and phase densities, the outer-iteration "
      "volumetric phi handoff is reconstructed from the exact VOF rhoPhi relation instead of "
      "being inferred from a weaker density ratio.");
  params.addParam<MooseFunctorName>(
      "liquid_density_functor",
      "rho_l",
      "Liquid-phase density functor used to reconstruct the VOF-consistent volumetric face flux "
      "from rhoPhi and alphaPhi during the outer-iteration handoff.");
  params.addParam<MooseFunctorName>(
      "gas_density_functor",
      "rho_g",
      "Gas-phase density functor used to reconstruct the VOF-consistent volumetric face flux "
      "from rhoPhi and alphaPhi during the outer-iteration handoff.");
  params.addParam<RealVectorValue>(
      "gravity",
      RealVectorValue(),
      "Gravity vector used by the reduced-pressure hydrostatic pressure initializer.");
  params.addParam<Point>(
      "reference_pressure_point",
      Point(0, 0, 0),
      "Reference point used to compute the reduced-pressure head gh = g.(x-x_ref).");
  params.set<MooseEnum>("pressure_projection_method") =
      MooseEnum("standard consistent", "consistent");

  return params;
}

SharpInterfaceRhieChowMassFlux::SharpInterfaceRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _transient_projection_flux(_moose_mesh, blockIDs(), "transient_projection_flux"),
    _capillary_hydrostatic_flux(_moose_mesh, blockIDs(), "capillary_hydrostatic_flux"),
    _pressure_Ainv(_moose_mesh, blockIDs(), "sharp_pressure_Ainv"),
    _predictor_convective_mass_flux(_moose_mesh, blockIDs(), "predictor_convective_mass_flux"),
    _predictor_convective_phi(_moose_mesh, blockIDs(), "predictor_convective_phi"),
    _predictor_operator_phi(_moose_mesh, blockIDs(), "predictor_operator_phi"),
    _pressure_predictor_base_phi(_moose_mesh, blockIDs(), "pressure_predictor_base_phi"),
    _pressure_equation_volumetric_flux(_moose_mesh, blockIDs(), "pressure_equation_volumetric_flux"),
    _pressure_correction_phi(_moose_mesh, blockIDs(), "pressure_correction_phi"),
    _sharp_pressure_predictor_flux(_moose_mesh, blockIDs(), "sharp_pressure_predictor_flux"),
    _reference_face_mass_flux_for_writeback(
        _moose_mesh, blockIDs(), "reference_face_mass_flux_for_writeback"),
    _corrected_face_phi(_moose_mesh, blockIDs(), "corrected_face_phi"),
    _outer_iteration_rho_phi(_moose_mesh, blockIDs(), "outer_iteration_rho_phi"),
    _outer_iteration_phi(_moose_mesh, blockIDs(), "outer_iteration_phi"),
    _pressure_coupled_velocity_correction_scalar(
        _moose_mesh, blockIDs(), "pressure_coupled_velocity_correction_scalar"),
    _pressure_coupled_velocity_correction_face(
        _moose_mesh, blockIDs(), "pressure_coupled_velocity_correction_face"),
    _corrected_face_velocity(_moose_mesh, blockIDs(), "corrected_face_velocity"),
    _add_transient_projection_flux(getParam<bool>("add_transient_projection_flux")),
    _add_capillary_hydrostatic_flux(getParam<bool>("add_capillary_hydrostatic_flux")),
    _use_face_based_predictor_body_force(getParam<bool>("use_face_based_predictor_body_force")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _transient_projection_face_acceleration_name(
        getParam<MooseFunctorName>("transient_projection_face_acceleration")),
    _surface_tension_face_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_face_acceleration")),
    _surface_tension_cell_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_cell_acceleration")),
    _hydrostatic_density_gradient_face_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration")),
    _hydrostatic_density_gradient_cell_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration")),
    _vof_rho_phi_name(getParam<MooseFunctorName>("vof_rho_phi_functor")),
    _vof_alpha_phi_limited_name(getParam<MooseFunctorName>("vof_alpha_phi_limited_functor")),
    _liquid_density_name(getParam<MooseFunctorName>("liquid_density_functor")),
    _gas_density_name(getParam<MooseFunctorName>("gas_density_functor")),
    _transient_projection_face_acceleration(
        _transient_projection_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_transient_projection_face_acceleration_name)),
    _surface_tension_face_acceleration(
        _surface_tension_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_surface_tension_face_acceleration_name)),
    _surface_tension_cell_acceleration(
        _surface_tension_cell_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_surface_tension_cell_acceleration_name)),
    _hydrostatic_density_gradient_face_acceleration(
        _hydrostatic_density_gradient_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_hydrostatic_density_gradient_face_acceleration_name)),
    _hydrostatic_density_gradient_cell_acceleration(
        _hydrostatic_density_gradient_cell_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_hydrostatic_density_gradient_cell_acceleration_name)),
    _vof_rho_phi(nullptr),
    _vof_alpha_phi_limited(nullptr),
    _liquid_density(nullptr),
    _gas_density(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("pressure_predictor_flux", _phiHbyA_flux, tid);
    UserObject::_subproblem.addFunctor("transient_projection_flux", _transient_projection_flux, tid);
    UserObject::_subproblem.addFunctor(
        "capillary_hydrostatic_flux", _capillary_hydrostatic_flux, tid);
    UserObject::_subproblem.addFunctor("sharp_pressure_Ainv", _pressure_Ainv, tid);
    UserObject::_subproblem.addFunctor(
        "predictor_convective_mass_flux", _predictor_convective_mass_flux, tid);
    UserObject::_subproblem.addFunctor("predictor_convective_phi", _predictor_convective_phi, tid);
    UserObject::_subproblem.addFunctor("predictor_operator_phi", _predictor_operator_phi, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_predictor_base_phi", _pressure_predictor_base_phi, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_equation_volumetric_flux", _pressure_equation_volumetric_flux, tid);
    UserObject::_subproblem.addFunctor("pressure_correction_phi", _pressure_correction_phi, tid);
    UserObject::_subproblem.addFunctor(
        "sharp_pressure_predictor_flux", _sharp_pressure_predictor_flux, tid);
    UserObject::_subproblem.addFunctor("corrected_face_phi", _corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor("pressure_coupled_velocity_correction_scalar",
                                       _pressure_coupled_velocity_correction_scalar,
                                       tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");

  if (splitMomentumPredictorOperator() &&
      !dynamic_cast<ReducedPressurePIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               ": split_momentum_predictor_operator is only supported with the "
               "ReducedPressurePIMPLE executioner, because that executioner is the one that "
               "adds the removed pressure/capillary momentum forcing back into the predictor. "
               "Use ReducedPressurePIMPLE or set split_momentum_predictor_operator = false.");

  rebuildSharpInterfaceFaceInfo();
  initializeAdditionalPressureFluxStorage();
}

Real
SharpInterfaceRhieChowMassFlux::getMassFlux(const FaceInfo & fi) const
{
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  const auto normalize_vof_rho_phi = [face_measure](const Real rho_phi)
  {
    return face_measure > libMesh::TOLERANCE ? rho_phi / face_measure : 0.0;
  };

  if (_use_vof_rho_phi)
  {
    if (_outer_iteration_convective_state_valid)
      return normalize_vof_rho_phi(libmesh_map_find(_outer_iteration_rho_phi, fi.id()));
    if (_vof_rho_phi)
      return normalize_vof_rho_phi(
          evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr));
  }

  return RhieChowMassFlux::getMassFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::rawRhieChowMassFlux(const FaceInfo & fi) const
{
  return RhieChowMassFlux::getMassFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::predictorOperatorFaceMassFlux(const FaceInfo & fi,
                                                              const Moose::StateArg & time_arg) const
{
  return transportMassFluxDensityFromVolumetricPhi(
      &fi, referenceFaceVelocityState(&fi, time_arg) * fi.normal(), time_arg);
}

Real
SharpInterfaceRhieChowMassFlux::pressureCoupledWritebackMassFlux(const FaceInfo & fi) const
{
  return pressureVelocityWritebackFluxDensity(&fi);
}

Real
SharpInterfaceRhieChowMassFlux::vofRhoPhiMassFlux(const FaceInfo & fi) const
{
  if (!_vof_rho_phi)
    return 0.0;

  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr) /
         face_measure;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::pressureCoupledCellVelocityDelta(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!_pressure_coupled_velocity_correction_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)
        ->updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  return reconstructPressureCoupledCellVelocityDelta(&elem_info, time_arg);
}

Real
SharpInterfaceRhieChowMassFlux::predictorVelocityComponent(const ElemInfo & elem_info,
                                                           const unsigned int component) const
{
  mooseAssert(component < _dim, "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return -(*_HbyA_raw[component])(dof);
}

void
SharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  if (!splitMomentumPredictorOperator())
    return;

  mooseAssert(system_i < _dim, "Momentum component index out of range in explicit forcing hook.");

  const auto time_arg = Moose::currentState();
  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  std::unique_ptr<FaceVectorField> predictor_pressure_force_face;
  bool have_face_based_predictor_pressure = false;
  if (_use_face_based_predictor_body_force)
  {
    predictor_body_force_face =
        std::make_unique<FaceVectorField>(_moose_mesh,
                                          blockIDs(),
                                          "momentum_predictor_body_force_face");
    populateMomentumPredictorBodyForceFaceField(*predictor_body_force_face, time_arg);
    predictor_pressure_force_face =
        std::make_unique<FaceVectorField>(_moose_mesh,
                                          blockIDs(),
                                          "momentum_predictor_pressure_force_face");
    have_face_based_predictor_pressure =
        const_cast<SharpInterfaceRhieChowMassFlux *>(this)->populateMomentumPredictorPressureForceFaceField(
            *predictor_pressure_force_face, time_arg);
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto * const elem = elem_info->elem();
    const auto elem_arg = makeElemArg(elem);
    const Real cell_volume = elem_info->volume() * elem_info->coordFactor();
    const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];

    Real rhs_contribution = 0.0;
    if (_use_face_based_predictor_body_force && have_face_based_predictor_pressure)
    {
      const RealVectorValue explicit_force_density =
          evaluateFaceBasedMomentumPredictorPressureForceDensity(
              elem_info, time_arg, predictor_pressure_force_face.get()) +
          (_add_capillary_hydrostatic_flux
               ? evaluateMomentumPredictorBodyForceDensity(
                     elem_info, time_arg, predictor_body_force_face.get())
               : RealVectorValue());
      rhs_contribution = explicit_force_density(system_i) * cell_volume;
    }
    else
    {
      rhs_contribution =
          -MetaPhysicL::raw_value(_p->gradient(elem_arg, time_arg))(system_i) * cell_volume;

      if (_add_capillary_hydrostatic_flux)
        rhs_contribution +=
            evaluateMomentumPredictorBodyForceDensity(
                elem_info, time_arg, predictor_body_force_face.get())(
                system_i) *
            cell_volume;
    }

    rhs.add(dof, rhs_contribution);
  }

  rhs.close();
}

void
SharpInterfaceRhieChowMassFlux::addMomentumPredictorBodyForceForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  if (!splitMomentumPredictorOperator())
    return;

  mooseAssert(system_i < _dim, "Momentum component index out of range in body-force forcing hook.");

  if (!_add_capillary_hydrostatic_flux)
  {
    rhs.close();
    return;
  }

  const auto time_arg = Moose::currentState();
  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  if (_use_face_based_predictor_body_force)
  {
    predictor_body_force_face =
        std::make_unique<FaceVectorField>(_moose_mesh,
                                          blockIDs(),
                                          "momentum_predictor_body_force_face");
    populateMomentumPredictorBodyForceFaceField(*predictor_body_force_face, time_arg);
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Real cell_volume = elem_info->volume() * elem_info->coordFactor();
    const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];
    const Real rhs_contribution =
        evaluateMomentumPredictorBodyForceDensity(
            elem_info, time_arg, predictor_body_force_face.get())(
            system_i) *
        cell_volume;

    rhs.add(dof, rhs_contribution);
  }

  rhs.close();
}

bool
SharpInterfaceRhieChowMassFlux::seedHydrostaticPressure(LinearSystem & pressure_system,
                                                        const dof_id_type pressure_pin_dof,
                                                        const Real pressure_pin_value) const
{
  static constexpr Real gravity_tol = 1e-12;
  static constexpr Real coordinate_tol = 1e-10;

  unsigned int dominant_component = libMesh::invalid_uint;
  Real dominant_magnitude = 0.0;
  for (const auto component : make_range(_dim))
  {
    const Real magnitude = std::abs(_gravity(component));
    if (magnitude > dominant_magnitude)
    {
      dominant_component = component;
      dominant_magnitude = magnitude;
    }
  }

  if (dominant_component == libMesh::invalid_uint || dominant_magnitude <= gravity_tol)
    return false;

  for (const auto component : make_range(_dim))
    if (component != dominant_component && std::abs(_gravity(component)) > gravity_tol)
      return false;

  struct LevelData
  {
    Real coordinate = 0.0;
    Real gh = 0.0;
    Real rho_sum = 0.0;
    unsigned int count = 0;
    std::vector<dof_id_type> dofs;
  };

  std::map<long long, LevelData> levels;
  long long pin_key = 0;
  bool found_pin = false;

  const auto time_arg = Moose::currentState();

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto * const elem = elem_info->elem();
    const auto elem_arg = makeElemArg(elem);
    const Point centroid = elem->vertex_average();
    const Real coordinate = centroid(dominant_component);
    const long long key = std::llround(coordinate / coordinate_tol);
    auto & level = levels[key];
    if (level.count == 0)
    {
      level.coordinate = coordinate;
      level.gh = _gravity * (centroid - _reference_pressure_point);
    }

    level.rho_sum += _rho(elem_arg, time_arg);
    level.count++;

    const auto dof = elem_info->dofIndices()[_global_pressure_system_number][0];
    level.dofs.push_back(dof);

    if (dof == pressure_pin_dof)
    {
      pin_key = key;
      found_pin = true;
    }
  }

  if (!found_pin || levels.empty())
    return false;

  struct ReducedPressureLevel
  {
    Real gh = 0.0;
    Real rho = 0.0;
    Real reduced_pressure = 0.0;
    std::vector<dof_id_type> dofs;
  };

  std::vector<std::pair<long long, ReducedPressureLevel>> ordered_levels;
  ordered_levels.reserve(levels.size());
  for (const auto & [key, level] : levels)
    ordered_levels.push_back(
        {key, {level.gh, level.rho_sum / level.count, 0.0, level.dofs}});

  std::sort(ordered_levels.begin(),
            ordered_levels.end(),
            [](const auto & lhs, const auto & rhs) { return lhs.second.gh < rhs.second.gh; });

  const auto pin_it = std::find_if(
      ordered_levels.begin(),
      ordered_levels.end(),
      [pin_key](const auto & level) { return level.first == pin_key; });
  if (pin_it == ordered_levels.end())
    return false;

  const auto pin_index = std::distance(ordered_levels.begin(), pin_it);
  pin_it->second.reduced_pressure = pressure_pin_value;

  Real total_pressure = pressure_pin_value + pin_it->second.rho * pin_it->second.gh;
  for (auto level_index = pin_index + 1; level_index < ordered_levels.size(); ++level_index)
  {
    const auto & previous = ordered_levels[level_index - 1].second;
    auto & current = ordered_levels[level_index].second;
    total_pressure += 0.5 * (previous.rho + current.rho) * (current.gh - previous.gh);
    current.reduced_pressure = total_pressure - current.rho * current.gh;
  }

  total_pressure = pressure_pin_value + pin_it->second.rho * pin_it->second.gh;
  for (int level_index = static_cast<int>(pin_index) - 1; level_index >= 0; --level_index)
  {
    const auto & next = ordered_levels[level_index + 1].second;
    auto & current = ordered_levels[level_index].second;
    total_pressure += 0.5 * (next.rho + current.rho) * (current.gh - next.gh);
    current.reduced_pressure = total_pressure - current.rho * current.gh;
  }

  auto & current_local_solution = *(pressure_system.system().current_local_solution);
  for (const auto & level : ordered_levels)
    for (const auto dof : level.second.dofs)
      current_local_solution.set(dof, level.second.reduced_pressure);

  current_local_solution.close();
  pressure_system.setSolution(current_local_solution);

  return true;
}

Real
SharpInterfaceRhieChowMassFlux::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  if (_use_vof_rho_phi && _outer_iteration_convective_state_valid)
    return libmesh_map_find(_outer_iteration_phi, fi.id());

  return libmesh_map_find(_corrected_face_phi, fi.id());
}

Real
SharpInterfaceRhieChowMassFlux::getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                                      const FaceInfo & fi,
                                                      const Moose::StateArg & time,
                                                      const THREAD_ID /*tid*/,
                                                      bool subtract_mesh_velocity) const
{
  mooseAssert(!subtract_mesh_velocity,
              "SharpInterfaceRhieChowMassFlux does not support moving meshes yet!");

  if (m != Moose::FV::InterpMethod::RhieChow)
    mooseError("Interpolation methods other than Rhie-Chow are not supported!");
  if (time.state != Moose::currentState().state)
    mooseError("Older interpolation times are not supported!");

  return getVolumetricFaceFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::maxVolumeFractionCourant(const Real dt) const
{
  if (dt <= 0.0)
    return 0.0;

  std::unordered_map<dof_id_type, Real> cell_flux_sum;
  cell_flux_sum.reserve(_sharp_interface_face_info.size());

  for (const auto * fi : _sharp_interface_face_info)
  {
    if (!fi)
      continue;

    const Real face_measure = fi->faceArea() * fi->faceCoord();
    const Real volumetric_flux = std::abs(getVolumetricFaceFlux(*fi)) * face_measure;

    if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
      cell_flux_sum[fi->elemPtr()->id()] += volumetric_flux;

    if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
      cell_flux_sum[fi->neighborPtr()->id()] += volumetric_flux;
  }

  Real max_courant = 0.0;
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Real volume = elem_info->volume() * elem_info->coordFactor();
    if (volume <= libMesh::TOLERANCE)
      continue;

    const auto it = cell_flux_sum.find(elem_info->elem()->id());
    if (it == cell_flux_sum.end())
      continue;

    max_courant = std::max(max_courant, 0.5 * dt * it->second / volume);
  }

  return max_courant;
}

void
SharpInterfaceRhieChowMassFlux::initializeAdditionalPressureFluxStorage(
    const bool preserve_corrected_face_phi)
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _transient_projection_flux[fi->id()] = 0.0;
    _capillary_hydrostatic_flux[fi->id()] = 0.0;
    _pressure_Ainv[fi->id()] = RealVectorValue();
    _predictor_convective_mass_flux[fi->id()] = 0.0;
    _predictor_convective_phi[fi->id()] = 0.0;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
    if (!preserve_corrected_face_phi)
      _corrected_face_phi[fi->id()] = 0.0;
    _outer_iteration_rho_phi[fi->id()] = 0.0;
    _outer_iteration_phi[fi->id()] = 0.0;
    _pressure_coupled_velocity_correction_scalar[fi->id()] = 0.0;
    _pressure_coupled_velocity_correction_face[fi->id()] = RealVectorValue();
    _corrected_face_velocity[fi->id()] = RealVectorValue();
  }

  _outer_iteration_convective_state_valid = false;
  _corrected_face_phi_seeded = preserve_corrected_face_phi && _corrected_face_phi_seeded;
  _pressure_coupled_velocity_correction_valid = false;
  _corrected_face_velocity_valid = false;
}

Real
SharpInterfaceRhieChowMassFlux::transportMassFluxDensityFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  const Real face_rho = interpolateFaceDensity(fi, time_arg);
  return face_rho * volumetric_phi;
}

Real
SharpInterfaceRhieChowMassFlux::transportIntegratedRhoPhiFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  const Real face_measure = fi->faceArea() * fi->faceCoord();
  return transportMassFluxDensityFromVolumetricPhi(fi, volumetric_phi, time_arg) * face_measure;
}

void
SharpInterfaceRhieChowMassFlux::cacheCurrentCorrectedVolumetricFlux()
{
  // Keep volumetric phi as the primary corrected face quantity. In the sharp
  // reduced-pressure path the pressure equation itself is assembled in
  // volumetric-flux space, so the final transported face flux is just the
  // predictor-plus-correction chain in that same operator space.
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    if (_pressure_predictor_face_state_valid && _pressure_equation_flux_valid)
    {
      _pressure_equation_volumetric_flux[fi->id()] = libmesh_map_find(_pressure_equation_flux, fi->id());
      _pressure_correction_phi[fi->id()] =
          libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
      _corrected_face_phi[fi->id()] = -libmesh_map_find(_predictor_operator_phi, fi->id()) +
                                      _pressure_correction_phi[fi->id()];
    }
    else
    {
      _pressure_equation_volumetric_flux[fi->id()] = 0.0;
      _pressure_correction_phi[fi->id()] = 0.0;
      const Real face_rho = predictorFaceDensity(fi, time_arg);
      _corrected_face_phi[fi->id()] =
          std::abs(face_rho) > libMesh::TOLERANCE
              ? libmesh_map_find(_face_mass_flux, fi->id()) / face_rho
              : 0.0;
    }
  }
  _corrected_face_phi_seeded = true;
}

void
SharpInterfaceRhieChowMassFlux::syncPredictorConvectiveStateFromCurrentFluxes()
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real paired_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    _predictor_convective_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(fi, paired_phi, time_arg);
    _predictor_convective_phi[fi->id()] = paired_phi;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
  }
}

void
SharpInterfaceRhieChowMassFlux::freezeOuterIterationConvectiveState()
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    Real paired_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    Real integrated_rho_phi =
        transportIntegratedRhoPhiFromVolumetricPhi(fi, paired_phi, time_arg);
    Real vof_integrated_rho_phi = integrated_rho_phi;
    if (_vof_rho_phi)
    {
      vof_integrated_rho_phi = evaluateFaceScalarFunctor(_vof_rho_phi, fi, time_arg, nullptr);
      integrated_rho_phi = vof_integrated_rho_phi;
    }

    const Real face_measure = fi->faceArea() * fi->faceCoord();
    if (_vof_rho_phi && _vof_alpha_phi_limited && _liquid_density && _gas_density &&
        face_measure > libMesh::TOLERANCE)
    {
      const Real alpha_phi_limited =
          evaluateFaceScalarFunctor(_vof_alpha_phi_limited, fi, time_arg, nullptr);
      const Real liquid_density = evaluateCellBasedFaceScalarFunctor(_liquid_density, fi, time_arg);
      const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, fi, time_arg);
      if (std::abs(gas_density) > libMesh::TOLERANCE)
        paired_phi =
            (vof_integrated_rho_phi - (liquid_density - gas_density) * alpha_phi_limited) /
            (gas_density * face_measure);
    }

    _outer_iteration_rho_phi[fi->id()] = integrated_rho_phi;
    _outer_iteration_phi[fi->id()] = paired_phi;
    _predictor_convective_mass_flux[fi->id()] =
        face_measure > libMesh::TOLERANCE ? integrated_rho_phi / face_measure : 0.0;
    _predictor_convective_phi[fi->id()] = paired_phi;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
  }

  _outer_iteration_convective_state_valid = true;
}

void
SharpInterfaceRhieChowMassFlux::clearOuterIterationConvectiveState()
{
  for (auto & pair : _outer_iteration_rho_phi)
    pair.second = 0.0;
  for (auto & pair : _outer_iteration_phi)
    pair.second = 0.0;
  for (auto & pair : _predictor_operator_phi)
    pair.second = 0.0;
  for (auto & pair : _pressure_predictor_base_phi)
    pair.second = 0.0;
  for (auto & pair : _pressure_equation_volumetric_flux)
    pair.second = 0.0;
  for (auto & pair : _pressure_correction_phi)
    pair.second = 0.0;
  for (auto & pair : _sharp_pressure_predictor_flux)
    pair.second = 0.0;
  for (auto & pair : _reference_face_mass_flux_for_writeback)
    pair.second = 0.0;

  _outer_iteration_convective_state_valid = false;
  syncPredictorConvectiveStateFromCurrentFluxes();
}

void
SharpInterfaceRhieChowMassFlux::updatePredictorOperatorPhiField(const Moose::StateArg & time_arg)
{
  for (const auto * fi : _sharp_interface_face_info)
    _predictor_operator_phi[fi->id()] = -(referenceFaceVelocityState(fi, time_arg) * fi->normal());
}

void
SharpInterfaceRhieChowMassFlux::rebuildSharpInterfaceFaceInfo()
{
  _sharp_interface_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _sharp_interface_face_info.push_back(fi);

  initializeAdditionalPressureFluxStorage();
}

void
SharpInterfaceRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  rebuildSharpInterfaceFaceInfo();
}

void
SharpInterfaceRhieChowMassFlux::initialSetup()
{
  RhieChowMassFlux::initialSetup();
  rebuildSharpInterfaceFaceInfo();
  if (!_vof_rho_phi_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_rho_phi_name, _tid))
    _vof_rho_phi = &getFunctor<Real>(_vof_rho_phi_name);
  if (!_vof_alpha_phi_limited_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_alpha_phi_limited_name, _tid))
    _vof_alpha_phi_limited = &getFunctor<Real>(_vof_alpha_phi_limited_name);
  if (!_liquid_density_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_liquid_density_name, _tid))
    _liquid_density = &getFunctor<Real>(_liquid_density_name);
  if (!_gas_density_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_gas_density_name, _tid))
    _gas_density = &getFunctor<Real>(_gas_density_name);
}

void
SharpInterfaceRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage(/*preserve_corrected_face_phi=*/true);
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();
}

void
SharpInterfaceRhieChowMassFlux::initFaceMassFlux()
{
  RhieChowMassFlux::initFaceMassFlux();
}

void
SharpInterfaceRhieChowMassFlux::computeFaceMassFlux()
{
  if (!_pressure_predictor_face_state_valid)
    updatePressurePredictorFaceState();

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  cacheCurrentCorrectedVolumetricFlux();

  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
    _face_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(
            fi, libmesh_map_find(_corrected_face_phi, fi->id()), time_arg);
}

Moose::FaceArg
SharpInterfaceRhieChowMassFlux::makeCenteredFaceArg(const FaceInfo * fi,
                                                    const Moose::StateArg * limiter_state) const
{
  return Moose::FaceArg{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, limiter_state};
}

Real
SharpInterfaceRhieChowMassFlux::interpolateFaceDensity(const FaceInfo * fi,
                                                       const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

    Real face_rho = 0.0;
    interpolate(InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
    return face_rho;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

  if (useConstrainedBoundaryPredictorState(fi))
  {
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
    return _rho(boundary_face, time_arg);
  }

  return _rho(makeElemArg(elem_info.elem()), time_arg);
}

Real
SharpInterfaceRhieChowMassFlux::predictorFaceDensity(const FaceInfo * fi,
                                                     const Moose::StateArg & time_arg) const
{
  // In the sharp-interface VOF path, rhoPhi is assembled from the primary volumetric
  // flux plus the limited alpha transport flux. It is therefore not generally equal
  // to rho_f * phi, and recovering a predictor "face density" from rhoPhi / phi
  // injects a non-physical variable-density mismatch into the pressure predictor and
  // velocity writeback chain.
  return interpolateFaceDensity(fi, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateFaceRawAinv(const FaceInfo * fi) const
{
  using namespace Moose::FV;

  RealVectorValue face_ainv;

  std::vector<PetscVectorReader> ainv_reader;
  ainv_reader.reserve(_Ainv_raw.size());
  for (const auto dim_i : index_range(_Ainv_raw))
    ainv_reader.emplace_back(*_Ainv_raw[dim_i]);

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];
    const auto neighbor_dof = neighbor_info.dofIndices()[_global_momentum_system_numbers[0]][0];

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  face_ainv(dim_i),
                  ainv_reader[dim_i](elem_dof),
                  ainv_reader[dim_i](neighbor_dof),
                  *fi,
                  true);
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
    const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[0]][0];

    for (const auto dim_i : make_range(_dim))
      face_ainv(dim_i) = ainv_reader[dim_i](elem_dof);
  }

  return face_ainv;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateFaceRau(const FaceInfo * fi) const
{
  using namespace Moose::FV;

  RealVectorValue face_rau;
  std::vector<PetscVectorReader> ainv_reader;
  ainv_reader.reserve(_Ainv_raw.size());
  for (const auto dim_i : index_range(_Ainv_raw))
    ainv_reader.emplace_back(*_Ainv_raw[dim_i]);

  auto cell_rau = [this, &ainv_reader](const ElemInfo & elem_info, const unsigned int dim_i)
  {
    const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    const Real cell_volume = elem_info.volume() * elem_info.coordFactor();
    return cell_volume > libMesh::TOLERANCE ? ainv_reader[dim_i](dof) / cell_volume : 0.0;
  };

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  face_rau(dim_i),
                  cell_rau(elem_info, dim_i),
                  cell_rau(neighbor_info, dim_i),
                  *fi,
                  true);
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
      face_rau(dim_i) = cell_rau(elem_info, dim_i);
  }

  return face_rau;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateFaceVectorFunctor(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return RealVectorValue();

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateBoundaryAwareVectorFunctor(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateCellVectorFunctorToFace(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto elem_value = MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const auto neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    return 0.5 * (elem_value + neighbor_value);
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateCellBodyForceDensityToFace(
    const Moose::Functor<RealVectorValue> * acceleration_functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!acceleration_functor)
    return RealVectorValue();

  const auto evaluate_cell_body_force = [this, &time_arg, acceleration_functor](const Elem * elem)
  {
    const auto elem_arg = makeElemArg(elem);
    const Real rho = _rho(elem_arg, time_arg);
    return rho * MetaPhysicL::raw_value((*acceleration_functor)(elem_arg, time_arg));
  };

  if (_vel[0]->isInternalFace(*fi))
    return 0.5 * (evaluate_cell_body_force(fi->elemPtr()) +
                  evaluate_cell_body_force(fi->neighborPtr()));

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return evaluate_cell_body_force(fluid_elem);
}

Real
SharpInterfaceRhieChowMassFlux::evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                                          const FaceInfo * fi,
                                                          const Moose::StateArg & time_arg,
                                                          const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
}

Real
SharpInterfaceRhieChowMassFlux::evaluateBoundaryAwareScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

Real
SharpInterfaceRhieChowMassFlux::evaluateCellBasedFaceScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor || !fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_value = MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const Real neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    return 0.5 * (elem_value + neighbor_value);
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

Real
SharpInterfaceRhieChowMassFlux::projectPhysicalMassFluxDensity(const Real face_rho,
                                                               const RealVectorValue & face_ainv_raw,
                                                               const RealVectorValue & face_acceleration,
                                                               const RealVectorValue & face_normal) const
{
  Real projected_flux = 0.0;
  for (const auto dim_i : make_range(_dim))
    projected_flux += face_rho * face_ainv_raw(dim_i) * face_acceleration(dim_i) * face_normal(dim_i);

  return projected_flux;
}

Real
SharpInterfaceRhieChowMassFlux::computeFaceNormalRawAinv(
    const RealVectorValue & face_ainv_raw, const RealVectorValue & face_normal) const
{
  Real normal_ainv = 0.0;
  for (const auto dim_i : make_range(_dim))
    normal_ainv += face_ainv_raw(dim_i) * face_normal(dim_i) * face_normal(dim_i);

  return normal_ainv;
}

Real
SharpInterfaceRhieChowMassFlux::massFluxDensityToVolumetricNormalFlux(
    const FaceInfo * fi, const Real mass_flux_density) const
{
  if (!fi)
    return 0.0;

  const RealVectorValue face_normal = fi->normal();
  const RealVectorValue face_raw_ainv = interpolateFaceRawAinv(fi);
  const RealVectorValue face_density_weighted_ainv = libmesh_map_find(_Ainv, fi->id());
  const Real normal_raw_ainv = computeFaceNormalRawAinv(face_raw_ainv, face_normal);
  const Real normal_density_weighted_ainv =
      computeFaceNormalRawAinv(face_density_weighted_ainv, face_normal);

  if (std::abs(normal_density_weighted_ainv) <= libMesh::TOLERANCE)
  {
    const Real face_rho = predictorFaceDensity(fi, Moose::currentState());
    return std::abs(face_rho) > libMesh::TOLERANCE ? mass_flux_density / face_rho : 0.0;
  }

  return mass_flux_density * normal_raw_ainv / normal_density_weighted_ainv;
}

Real
SharpInterfaceRhieChowMassFlux::computeFaceNormalDensityGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Real normal_spacing = std::abs(fi->dCN() * fi->normal());

    return normal_spacing > libMesh::TOLERANCE ? (neighbor_rho - elem_rho) / normal_spacing : 0.0;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Point fluid_centroid = elem_is_fluid ? fi->elemCentroid() : fi->neighborCentroid();
  const Point cell_to_face = fi->faceCentroid() - fluid_centroid;
  const Real normal_spacing = std::abs(cell_to_face * fi->normal());

  if (normal_spacing <= libMesh::TOLERANCE)
    return 0.0;

  const Real elem_rho = _rho(makeElemArg(fluid_elem), time_arg);
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, fluid_elem, nullptr};
  const Real boundary_rho = _rho(boundary_face, time_arg);

  return (boundary_rho - elem_rho) / normal_spacing;
}

Real
SharpInterfaceRhieChowMassFlux::computeHydrostaticFaceMassFlux(
    const FaceInfo * fi,
    const Real face_rho,
    const RealVectorValue & face_ainv_raw,
    const RealVectorValue & face_normal,
    const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(*fi))
  {
    _p_diffusion_kernel->setupFaceData(fi);
    _p_diffusion_kernel->setCurrentFaceArea(1.0);

    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Real density_jump = neighbor_rho - elem_rho;
    const Real gh = _gravity * (fi->faceCentroid() - _reference_pressure_point);
    const Real pressure_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();

    return -pressure_matrix_contribution * gh * density_jump;
  }

  const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
  if (std::abs(sn_grad_rho) <= libMesh::TOLERANCE)
    return 0.0;

  const Point x = fi->faceCentroid();
  const Real gh = _gravity * (x - _reference_pressure_point);

  RealVectorValue hydrostatic_accel =
      -(gh / std::max(face_rho, libMesh::TOLERANCE)) * sn_grad_rho * face_normal;
  return projectPhysicalMassFluxDensity(face_rho, face_ainv_raw, hydrostatic_accel, face_normal);
}

bool
SharpInterfaceRhieChowMassFlux::populateMomentumPredictorBodyForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg) const
{
  const bool have_surface_face_force = _surface_tension_face_acceleration;
  const bool have_hydro_force =
      _hydrostatic_density_gradient_face_acceleration ||
      _hydrostatic_density_gradient_cell_acceleration;

  if (!have_surface_face_force && !have_hydro_force)
    return false;

  for (const auto * fi : _sharp_interface_face_info)
  {
    const RealVectorValue face_normal = fi->normal();
    RealVectorValue face_body_force_density;

    if (have_surface_face_force)
    {
      const Real face_rho = interpolateFaceDensity(fi, time_arg);
      const auto surface_accel =
          evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
      face_body_force_density += face_rho * surface_accel;
    }
    else if (_surface_tension_cell_acceleration)
    {
      const auto surface_force_density =
          interpolateCellBodyForceDensityToFace(_surface_tension_cell_acceleration, fi, time_arg);
      face_body_force_density += (surface_force_density * face_normal) * face_normal;
    }

    if (have_hydro_force)
    {
      const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
      const Real gh = _gravity * (fi->faceCentroid() - _reference_pressure_point);
      face_body_force_density += -(gh * sn_grad_rho) * face_normal;
    }

    const Real normal_force_density = face_body_force_density * face_normal;
    face_field[fi->id()] = normal_force_density * face_normal;
  }

  return true;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateLegacyMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);
  const Real rho = _rho(elem_arg, time_arg);

  if (_surface_tension_cell_acceleration)
    body_force_density +=
        rho * MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg));

  if (_hydrostatic_density_gradient_cell_acceleration)
    body_force_density +=
        rho *
        MetaPhysicL::raw_value((*_hydrostatic_density_gradient_cell_acceleration)(elem_arg, time_arg));

  return body_force_density;
}

bool
SharpInterfaceRhieChowMassFlux::populateMomentumPredictorPressureForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg)
{
  (void)time_arg;
  cachePressureEquationFlux();

  bool found_nonzero_force = false;
  for (const auto * fi : _sharp_interface_face_info)
  {
    const RealVectorValue face_normal = fi->normal();

    Real negative_sn_grad_p = 0.0;
    if (!_vel[0]->isInternalFace(*fi) && _pressure_boundary_normal_gradient_valid)
      negative_sn_grad_p = -libmesh_map_find(_pressure_boundary_normal_gradient, fi->id());
    else
    {
      const auto & face_ainv = libmesh_map_find(_Ainv, fi->id());
      Real normal_ainv = 0.0;
      for (const auto dim_i : make_range(_dim))
        normal_ainv += face_ainv(dim_i) * face_normal(dim_i) * face_normal(dim_i);

      if (std::abs(normal_ainv) > libMesh::TOLERANCE)
        negative_sn_grad_p = libmesh_map_find(_pressure_equation_flux, fi->id()) / normal_ainv;
    }

    if (std::abs(negative_sn_grad_p) > libMesh::TOLERANCE)
      found_nonzero_force = true;

    face_field[fi->id()] = negative_sn_grad_p * face_normal;
  }

  return found_nonzero_force;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateFaceBasedMomentumPredictorPressureForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (!elem_info || !face_field)
    return RealVectorValue();

  return (*face_field)(makeElemArg(elem_info->elem()), time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateFaceBasedMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);
  const Real rho = _rho(elem_arg, time_arg);

  if (face_field)
    body_force_density += (*face_field)(elem_arg, time_arg);

  if (!_surface_tension_face_acceleration && _surface_tension_cell_acceleration)
    body_force_density +=
        rho * MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg));

  if (!_hydrostatic_density_gradient_face_acceleration &&
      _hydrostatic_density_gradient_cell_acceleration)
    body_force_density +=
        rho *
        MetaPhysicL::raw_value((*_hydrostatic_density_gradient_cell_acceleration)(elem_arg, time_arg));

  return body_force_density;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (_use_face_based_predictor_body_force && face_field)
    return evaluateFaceBasedMomentumPredictorBodyForceDensity(elem_info, time_arg, face_field);

  return evaluateLegacyMomentumPredictorBodyForceDensity(elem_info, time_arg);
}

Real
SharpInterfaceRhieChowMassFlux::pressureVelocityWritebackFluxDensity(const FaceInfo * fi) const
{
  const Real correction_phi =
      libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
  return transportMassFluxDensityFromVolumetricPhi(fi, correction_phi, Moose::currentState());
}

void
SharpInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors(const bool with_updated_pressure,
                                                                     const bool verbose)
{
  (void)with_updated_pressure;

  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;
  _corrected_face_velocity_valid = false;

  const auto time_arg = Moose::currentState();
  updatePredictorOperatorPhiField(time_arg);

  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real face_rho = predictorFaceDensity(fi, time_arg);
    const RealVectorValue face_ainv_raw = interpolateFaceRawAinv(fi);
    const RealVectorValue face_normal = fi->normal();
    const Real predictor_operator_phi = _predictor_operator_phi[fi->id()];
    const Real predictor_operator_mass_flux =
        transportMassFluxDensityFromVolumetricPhi(fi, -predictor_operator_phi, time_arg);

    Real physical_transient_flux = 0.0;
    if (!_suppress_startup_pressure_predictor_flux_sources && _add_transient_projection_flux &&
        _transient_projection_face_acceleration)
    {
      const auto transient_accel =
          evaluateBoundaryAwareVectorFunctor(_transient_projection_face_acceleration, fi, time_arg);
      physical_transient_flux =
          projectPhysicalMassFluxDensity(face_rho, face_ainv_raw, transient_accel, face_normal);
    }

    RealVectorValue capillary_hydrostatic_accel;
    if (_add_capillary_hydrostatic_flux)
    {
      if (_surface_tension_face_acceleration)
        capillary_hydrostatic_accel +=
            evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
    }

    Real physical_capillary_hydrostatic_flux =
        projectPhysicalMassFluxDensity(face_rho,
                                       face_ainv_raw,
                                       capillary_hydrostatic_accel,
                                       face_normal);
    if (!_suppress_startup_pressure_predictor_flux_sources && _add_capillary_hydrostatic_flux &&
        _hydrostatic_density_gradient_face_acceleration &&
        !_suppress_explicit_hydrostatic_pressure_flux)
      physical_capillary_hydrostatic_flux +=
          computeHydrostaticFaceMassFlux(fi, face_rho, face_ainv_raw, face_normal, time_arg);

    _pressure_Ainv[fi->id()] = face_ainv_raw;

    // Publish pressure-equation source fluxes in volumetric-flux units so the
    // sharp reduced-pressure pressure equation, boundary constraints, and final
    // transported phi all live in the same operator space.
    _transient_projection_flux[fi->id()] =
        -massFluxDensityToVolumetricNormalFlux(fi, physical_transient_flux);
    _capillary_hydrostatic_flux[fi->id()] =
        -massFluxDensityToVolumetricNormalFlux(fi, physical_capillary_hydrostatic_flux);
    _phig_flux[fi->id()] =
        _transient_projection_flux[fi->id()] + _capillary_hydrostatic_flux[fi->id()];
    _predictor_operator_phi[fi->id()] = predictor_operator_phi;

    // Keep the carried outer-iteration phi branch for alpha transport, but use
    // the operator-built face state for the pressure predictor. The final cell
    // writeback is now driven by a matched scalar correction solve plus the
    // pressure-coupled reconstruction, so the pressure-side predictor branch
    // must stay on the directly assembled operator state here.
    const Real sharp_predictor_phi = predictor_operator_phi;
    _sharp_pressure_predictor_flux[fi->id()] = sharp_predictor_phi + _phig_flux[fi->id()];
    _pressure_predictor_base_phi[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _pressure_predictor_base_flux[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _phiHbyA_flux[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _reference_face_mass_flux_for_writeback[fi->id()] = predictor_operator_mass_flux;

    if (verbose)
    {
      _console << "Sharp-interface predictor on face " << fi->id() << ": transient_source_flux="
               << _transient_projection_flux[fi->id()]
               << ", capillary_hydrostatic_source_flux="
               << _capillary_hydrostatic_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
               << ", predictor_operator_phi=" << _predictor_operator_phi[fi->id()]
               << ", sharp_predictor_phi=" << sharp_predictor_phi
               << ", pressure_predictor_base_phi=" << _pressure_predictor_base_phi[fi->id()]
               << ", phiHbyA_flux=" << _phiHbyA_flux[fi->id()]
               << std::endl;
    }
  }

  _pressure_predictor_face_state_valid = true;
}

void
SharpInterfaceRhieChowMassFlux::updatePressureCoupledVelocityCorrectionFaceField(
    const Moose::StateArg & time_arg)
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  for (const auto * fi : _sharp_interface_face_info)
  {
    // The live writeback must consume the same physical pressure-correction
    // branch that the reduced-pressure face update and consistency audits use:
    //   pressure_equation_flux - phig_flux.
    // Using the total final-minus-reference face increment here mixes the
    // pressure branch with the predictor/cell-face mismatch, which is a
    // different discrete error source.
    const Real pressure_coupled_phi =
        libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
    const RealVectorValue face_normal = fi->normal();

    // Keep the scalar correction for audits. This is the face-normal velocity
    // correction implied directly by the physical pressure-correction branch.
    const Real psi_f = pressure_coupled_phi;
    _pressure_coupled_velocity_correction_scalar[fi->id()] = psi_f;

    // Keep phi authoritative and publish only the physical pressure-correction
    // flux branch for the final cell writeback:
    //   pressureVelocityWritebackFluxDensity = pressure_equation_flux - phig_flux.
    // Do not mix any additional explicit source terms back into the writeback here.
    // They already entered the physical pressure/face-flux update through phiHbyA/phig.
    _pressure_coupled_velocity_correction_face[fi->id()] = psi_f * face_normal;
  }

  _pressure_coupled_velocity_correction_valid = true;
}

bool
SharpInterfaceRhieChowMassFlux::useConstrainedBoundaryPredictorState(const FaceInfo * fi) const
{
  if (!fi || _vel[0]->isInternalFace(*fi))
    return false;

  bool use_constrained_boundary_state = _vel[0]->isDirichletBoundaryFace(*fi);
  if (!use_constrained_boundary_state && !fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1,
                "Expected at most one physical boundary id on a FV boundary face.");
    const auto boundary_id = *fi->boundaryIDs().begin();

    for (const auto dim_i : make_range(_dim))
      if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(boundary_id))
        if (auto * pressure_inlet_outlet_bc =
                dynamic_cast<LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
        {
          pressure_inlet_outlet_bc->setupFaceData(
              fi,
              fi->faceType(std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          if (pressure_inlet_outlet_bc->computeBoundaryGradientMatrixContribution() > 0.0)
          {
            use_constrained_boundary_state = true;
            break;
          }
        }
  }

  return use_constrained_boundary_state;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::referenceFaceVelocityState(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  RealVectorValue face_velocity;

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  face_velocity(dim_i),
                  predictorVelocityComponent(elem_info, dim_i),
                  predictorVelocityComponent(neighbor_info, dim_i),
                  *fi,
                  true);

    return face_velocity;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

  if (useConstrainedBoundaryPredictorState(fi))
  {
    for (const auto dim_i : make_range(_dim))
    {
      Real boundary_value = std::numeric_limits<Real>::quiet_NaN();

      if (!fi->boundaryIDs().empty())
      {
        mooseAssert(fi->boundaryIDs().size() == 1,
                    "Expected at most one physical boundary id on a FV boundary face.");
        if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(*fi->boundaryIDs().begin()))
        {
          bc_pointer->setupFaceData(
              fi,
              fi->faceType(
                  std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          boundary_value = bc_pointer->computeBoundaryValue();
        }
      }

      face_velocity(dim_i) =
          std::isfinite(boundary_value)
              ? boundary_value
              : MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, time_arg));
    }
  }
  else
    for (const auto dim_i : make_range(_dim))
      face_velocity(dim_i) = predictorVelocityComponent(elem_info, dim_i);

  return face_velocity;
}

Real
SharpInterfaceRhieChowMassFlux::referenceFaceMassFluxState(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    RealVectorValue density_times_velocity;

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  density_times_velocity(dim_i),
                  elem_rho * predictorVelocityComponent(elem_info, dim_i),
                  neighbor_rho * predictorVelocityComponent(neighbor_info, dim_i),
                  *fi,
                  true);

    return density_times_velocity * fi->normal();
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  if (useConstrainedBoundaryPredictorState(fi))
  {
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);
    const RealVectorValue predictor_face_velocity = referenceFaceVelocityState(fi, time_arg);
    return boundary_normal_multiplier * face_rho * (predictor_face_velocity * fi->normal());
  }

  const Real cell_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
  RealVectorValue density_times_velocity;
  for (const auto dim_i : make_range(_dim))
    density_times_velocity(dim_i) =
        boundary_normal_multiplier * cell_rho * predictorVelocityComponent(elem_info, dim_i);

  return density_times_velocity * fi->normal();
}

void
SharpInterfaceRhieChowMassFlux::updateCorrectedFaceVelocityField(const Moose::StateArg & time_arg)
{
  for (const auto * fi : _sharp_interface_face_info)
  {
    const RealVectorValue face_normal = fi->normal();
    const RealVectorValue reference_face_velocity = referenceFaceVelocityState(fi, time_arg);
    const Real reference_normal_velocity = reference_face_velocity * face_normal;
    const RealVectorValue reference_tangential_velocity =
        reference_face_velocity - reference_normal_velocity * face_normal;

    const Real corrected_normal_velocity = libmesh_map_find(_corrected_face_phi, fi->id());

    _corrected_face_velocity[fi->id()] =
        reference_tangential_velocity + corrected_normal_velocity * face_normal;
  }

  _corrected_face_velocity_valid = true;
}

bool
SharpInterfaceRhieChowMassFlux::shouldUseCorrectedBoundaryVelocityState(const FaceInfo * fi) const
{
  if (!_corrected_face_velocity_valid || !fi || _vel[0]->isInternalFace(*fi) || fi->boundaryIDs().empty())
    return false;

  mooseAssert(fi->boundaryIDs().size() == 1, "Expected a single boundary id on a FV boundary face.");
  const auto boundary_id = *fi->boundaryIDs().begin();

  for (const auto component : make_range(_dim))
    if (const auto * bc_pointer = _vel[component]->getBoundaryCondition(boundary_id);
        dynamic_cast<const LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
      return true;

  return false;
}

Real
SharpInterfaceRhieChowMassFlux::pressureBoundaryTargetFlux(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  if (!fi || _vel[0]->isInternalFace(*fi))
    return 0.0;

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
  const RealVectorValue predictor_face_velocity = referenceFaceVelocityState(fi, time_arg);
  return boundary_normal_multiplier * (predictor_face_velocity * fi->normal());
}

Real
SharpInterfaceRhieChowMassFlux::pressureBoundaryNormalAinv(const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  return computeFaceNormalRawAinv(interpolateFaceRawAinv(fi), fi->normal());
}

void
SharpInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
{
  RhieChowMassFlux::updateVelocityBoundaryState();

  if (!_corrected_face_velocity_valid)
    return;

  const auto time_arg = Moose::currentState();

  for (const auto * fi : flowFaceInfo())
  {
    if (!shouldUseCorrectedBoundaryVelocityState(fi))
      continue;

    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);
    const RealVectorValue corrected_face_velocity =
        libmesh_map_find(_corrected_face_velocity, fi->id());

    RealVectorValue density_times_velocity;
    for (const auto component : make_range(_dim))
    {
      _boundary_velocity_face_values[component][fi->id()] = corrected_face_velocity(component);
      density_times_velocity(component) =
          boundary_normal_multiplier * face_rho * corrected_face_velocity(component);
    }

    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  cacheCurrentCorrectedVolumetricFlux();
  _velocity_boundary_state_valid = true;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructMatchedPressureCoupledCellCorrectionSource(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();

  const Elem * const elem = elem_info->elem();
  if (!elem)
    return RealVectorValue();

  DenseMatrix<Real> normal_matrix(_dim, _dim);
  DenseVector<Real> rhs(_dim);
  DenseVector<Real> solution(_dim);

  for (const auto i : make_range(_dim))
  {
    rhs(i) = 0.0;
    solution(i) = 0.0;
    for (const auto j : make_range(_dim))
      normal_matrix(i, j) = 0.0;
  }

  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    const Real weight = std::max(fi_loc->faceArea() * fi_loc->faceCoord(), libMesh::TOLERANCE);
    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const Real psi_f =
        _pressure_coupled_velocity_correction_scalar(makeCenteredFaceArg(fi_loc), time_arg);

    for (const auto i : make_range(_dim))
    {
      rhs(i) += weight * outward_normal(i) * psi_f;
      for (const auto j : make_range(_dim))
        normal_matrix(i, j) += weight * outward_normal(i) * outward_normal(j);
    }
  }

  bool singular = false;
  if (_dim == 1)
    singular = std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 2)
    singular = std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                        normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 3)
  {
    const Real det =
        normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 1)) -
        normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 0)) +
        normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                               normal_matrix(1, 1) * normal_matrix(2, 0));
    singular = std::abs(det) <= libMesh::TOLERANCE;
  }

  if (singular)
    return _pressure_coupled_velocity_correction_face(makeElemArg(elem), time_arg);

  normal_matrix.lu_solve(rhs, solution);
  RealVectorValue reconstructed_source;
  for (const auto i : make_range(_dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellCorrectionSource(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  return reconstructMatchedPressureCoupledCellCorrectionSource(elem_info, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::matchedSourcePressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue delta_velocity;

  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return delta_velocity;

  const RealVectorValue correction_source =
      reconstructPressureCoupledCellCorrectionSource(elem_info, time_arg);

  for (const auto dim_i : make_range(_dim))
  {
    const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    delta_velocity(dim_i) = (*_Ainv_raw[dim_i])(dof) * correction_source(dim_i);
  }

  return delta_velocity;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();

  return matchedSourcePressureCoupledCellVelocityDelta(elem_info, time_arg);
}

void
SharpInterfaceRhieChowMassFlux::applyAdditionalFaceMassFluxCorrection()
{
  // computeFaceMassFlux() now uses the explicit phiHbyA face state, which in the
  // sharp-interface path already includes the transient and capillary/hydrostatic
  // predictor-source fluxes.
  // There is therefore no additional post-solve face-flux correction to apply here.
}

void
SharpInterfaceRhieChowMassFlux::computeProvisionalCellVelocity()
{
  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  _corrected_face_velocity_valid = false;
  updateCorrectedFaceVelocityField(time_arg);

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const RealVectorValue delta_velocity =
        reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);
    RealVectorValue corrected_cell_velocity;
    for (const auto dim_i : make_range(_dim))
    {
      corrected_cell_velocity(dim_i) =
          predictorVelocityComponent(*elem_info, dim_i) + delta_velocity(dim_i);
    }

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      auto & solution = *(_momentum_implicit_systems[system_i]->solution);
      const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];
      solution.set(dof, corrected_cell_velocity(system_i));
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto & solution = *(_momentum_implicit_systems[system_i]->solution);
    solution.close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
SharpInterfaceRhieChowMassFlux::computeCellVelocity()
{
  computeProvisionalCellVelocity();
}
