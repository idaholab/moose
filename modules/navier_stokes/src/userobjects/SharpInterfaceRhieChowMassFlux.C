#include "SharpInterfaceRhieChowMassFlux.h"

#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "PIMPLE.h"
#include "ReducedPressurePIMPLE.h"
#include "SIMPLE.h"
#include "SubProblem.h"

#include <algorithm>
#include <cmath>
#include <map>
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
    _pressure_coupled_velocity_correction_face(
        _moose_mesh, blockIDs(), "pressure_coupled_velocity_correction_face"),
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
    _vof_rho_phi(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("pressure_predictor_flux", _phiHbyA_flux, tid);
    UserObject::_subproblem.addFunctor("transient_projection_flux", _transient_projection_flux, tid);
    UserObject::_subproblem.addFunctor(
        "capillary_hydrostatic_flux", _capillary_hydrostatic_flux, tid);
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
  if (_use_vof_rho_phi && _vof_rho_phi)
    return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr);

  return RhieChowMassFlux::getMassFlux(fi);
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
  if (_use_vof_rho_phi && _vof_rho_phi)
  {
    const auto face_arg = makeCenteredFaceArg(&fi);
    const Real face_rho = _rho(face_arg, Moose::currentState());
    return face_rho > libMesh::TOLERANCE ? getMassFlux(fi) / face_rho : 0.0;
  }

  return RhieChowMassFlux::getVolumetricFaceFlux(fi);
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

void
SharpInterfaceRhieChowMassFlux::initializeAdditionalPressureFluxStorage()
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _transient_projection_flux[fi->id()] = 0.0;
    _capillary_hydrostatic_flux[fi->id()] = 0.0;
    _pressure_coupled_velocity_correction_face[fi->id()] = RealVectorValue();
  }

  _pressure_coupled_velocity_correction_valid = false;
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
}

void
SharpInterfaceRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage();
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

  if (_vel[0]->isDirichletBoundaryFace(*fi))
  {
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
    return _rho(boundary_face, time_arg);
  }

  return _rho(makeElemArg(elem_info.elem()), time_arg);
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

void
SharpInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors(const bool with_updated_pressure,
                                                                     const bool verbose)
{
  (void)with_updated_pressure;

  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;

  const auto time_arg = Moose::currentState();

  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real face_rho = interpolateFaceDensity(fi, time_arg);
    const RealVectorValue face_ainv_raw = interpolateFaceRawAinv(fi);
    const RealVectorValue face_normal = fi->normal();

    Real physical_transient_flux = 0.0;
    if (_add_transient_projection_flux && _transient_projection_face_acceleration)
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
    if (_add_capillary_hydrostatic_flux && _hydrostatic_density_gradient_face_acceleration &&
        !_suppress_explicit_hydrostatic_pressure_flux)
      physical_capillary_hydrostatic_flux +=
          computeHydrostaticFaceMassFlux(fi, face_rho, face_ainv_raw, face_normal, time_arg);

    // Publish pressure-equation source fluxes in the same sign convention as HbyA,
    // not the physical face mass flux. The physical correction is therefore the
    // negative of the stored value.
    _transient_projection_flux[fi->id()] = -physical_transient_flux;
    _capillary_hydrostatic_flux[fi->id()] = -physical_capillary_hydrostatic_flux;
    _phig_flux[fi->id()] =
        _transient_projection_flux[fi->id()] + _capillary_hydrostatic_flux[fi->id()];
    _pressure_predictor_base_flux[fi->id()] = _HbyA_flux[fi->id()] + _phig_flux[fi->id()];
    _phiHbyA_flux[fi->id()] = _pressure_predictor_base_flux[fi->id()];

    if (verbose)
    {
      _console << "Sharp-interface predictor on face " << fi->id() << ": transient_source_flux="
               << _transient_projection_flux[fi->id()]
               << ", capillary_hydrostatic_source_flux="
               << _capillary_hydrostatic_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
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
    const Real pressure_coupled_flux_density =
        libmesh_map_find(_pressure_equation_flux, fi->id()) -
        libmesh_map_find(_capillary_hydrostatic_flux, fi->id());

    if (std::abs(pressure_coupled_flux_density) <= libMesh::TOLERANCE)
    {
      _pressure_coupled_velocity_correction_face[fi->id()] = RealVectorValue();
      continue;
    }

    const RealVectorValue face_normal = fi->normal();
    const Real face_rho = interpolateFaceDensity(fi, time_arg);
    const RealVectorValue face_ainv_raw = interpolateFaceRawAinv(fi);
    const Real normal_ainv = computeFaceNormalRawAinv(face_ainv_raw, face_normal);
    const Real denom = face_rho * normal_ainv;

    if (std::abs(denom) <= libMesh::TOLERANCE)
    {
      _pressure_coupled_velocity_correction_face[fi->id()] = RealVectorValue();
      continue;
    }

    const Real normal_acceleration = pressure_coupled_flux_density / denom;
    _pressure_coupled_velocity_correction_face[fi->id()] = normal_acceleration * face_normal;
  }

  _pressure_coupled_velocity_correction_valid = true;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue delta_velocity;

  if (!elem_info)
    return delta_velocity;

  if (!_pressure_coupled_velocity_correction_valid)
    return delta_velocity;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);
  const RealVectorValue reconstructed_acceleration =
      _pressure_coupled_velocity_correction_face(elem_arg, time_arg);

  if (reconstructed_acceleration.norm() <= libMesh::TOLERANCE)
    return delta_velocity;

  for (const auto dim_i : make_range(_dim))
  {
    const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    delta_velocity(dim_i) = (*_Ainv_raw[dim_i])(dof) * reconstructed_acceleration(dim_i);
  }

  return delta_velocity;
}

void
SharpInterfaceRhieChowMassFlux::auditRepresentativeHorizontalFaceReconstruction()
{
  if (!_pressure_coupled_velocity_correction_valid)
    updatePressureCoupledVelocityCorrectionFaceField(Moose::currentState());

  static unsigned int audit_counter = 0;
  ++audit_counter;

  const FaceInfo * target_face = nullptr;
  const FaceInfo * target_bottom_face = nullptr;
  const FaceInfo * target_top_face = nullptr;
  Real best_metric = std::numeric_limits<Real>::max();
  Real best_bottom_metric = std::numeric_limits<Real>::max();
  Real best_top_metric = std::numeric_limits<Real>::max();
  const ElemInfo * target_liquid_elem_info = nullptr;
  const ElemInfo * target_gas_elem_info = nullptr;
  Real best_liquid_metric = std::numeric_limits<Real>::max();
  Real best_gas_metric = std::numeric_limits<Real>::max();

  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto normal = fi->normal();
    const auto centroid = fi->faceCentroid();

    if (_vel[0]->isInternalFace(*fi) && std::abs(normal(1)) >= 0.999)
    {
      const Real metric = std::abs(centroid(1) - 0.5) + 0.1 * std::abs(centroid(0) - 0.25);
      if (metric < best_metric)
      {
        best_metric = metric;
        target_face = fi;
      }
    }
    else if (!_vel[0]->isInternalFace(*fi) && std::abs(normal(1)) >= 0.999)
    {
      const Real bottom_metric = std::abs(centroid(1)) + 0.1 * std::abs(centroid(0) - 0.25);
      if (bottom_metric < best_bottom_metric)
      {
        best_bottom_metric = bottom_metric;
        target_bottom_face = fi;
      }

      const Real top_metric = std::abs(centroid(1) - 1.0) + 0.1 * std::abs(centroid(0) - 0.25);
      if (top_metric < best_top_metric)
      {
        best_top_metric = top_metric;
        target_top_face = fi;
      }
    }
  }

  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  auto audit_internal_face = [this, &p_reader](const FaceInfo * face)
  {
    if (!face)
      return;

    const auto face_id = face->id();
    _p_diffusion_kernel->setupFaceData(face);
    _p_diffusion_kernel->setCurrentFaceArea(1.0);

    const auto & elem_info = *face->elemInfo();
    const auto & neighbor_info = *face->neighborInfo();
    const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
    const auto neighbor_dof = neighbor_info.dofIndices()[_global_pressure_system_number][0];
    const Real p_elem_value = p_reader(elem_dof);
    const Real p_neighbor_value = p_reader(neighbor_dof);
    const Real elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
    const Real neighbor_matrix_contribution =
        _p_diffusion_kernel->computeNeighborMatrixContribution();
    const Real elem_rhs_contribution =
        _p_diffusion_kernel->computeElemRightHandSideContribution();
    const Real reconstructed_p_grad_flux =
        p_neighbor_value * neighbor_matrix_contribution +
        p_elem_value * elem_matrix_contribution - elem_rhs_contribution;

    const Real hbya_source = libmesh_map_find(_HbyA_flux, face_id);
    const Real transient_source = libmesh_map_find(_transient_projection_flux, face_id);
    const Real capillary_hydrostatic_source =
        libmesh_map_find(_capillary_hydrostatic_flux, face_id);
    const Real final_face_flux = _face_mass_flux[face_id];
    const Real source_sum = hbya_source + transient_source + capillary_hydrostatic_source;
    const Real inferred_p_grad_flux = final_face_flux + source_sum;

    _console << "Sharp-interface face audit: audit_step=" << audit_counter
             << ", face_id=" << face_id
             << ", centroid=" << face->faceCentroid()
             << ", normal=" << face->normal()
             << ", p_elem=" << p_elem_value
             << ", p_neighbor=" << p_neighbor_value
             << ", elem_matrix=" << elem_matrix_contribution
             << ", neighbor_matrix=" << neighbor_matrix_contribution
             << ", elem_rhs=" << elem_rhs_contribution
             << ", reconstructed_p_grad_flux=" << reconstructed_p_grad_flux
             << ", HbyA_source=" << hbya_source
             << ", transient_source=" << transient_source
             << ", capillary_hydrostatic_source=" << capillary_hydrostatic_source
             << ", source_sum=" << source_sum
             << ", inferred_p_grad_flux=" << inferred_p_grad_flux
             << ", final_face_flux=" << final_face_flux << std::endl;
  };

  const auto time_arg = Moose::currentState();
  auto audit_boundary_face = [this, &time_arg](const FaceInfo * face, const std::string & label)
  {
    if (!face || face->boundaryIDs().empty())
      return;

    auto * bc_pointer = _p->getBoundaryCondition(*face->boundaryIDs().begin());
    if (!bc_pointer)
      return;

    mooseAssert(face->boundaryIDs().size() == 1, "Expected a single boundary id on wall face.");
    _p_diffusion_kernel->setupFaceData(face);
    _p_diffusion_kernel->setCurrentFaceArea(1.0);
    bc_pointer->setupFaceData(
        face, face->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

    const bool elem_is_fluid = hasBlocks(face->elemPtr()->subdomain_id());
    const ElemInfo & fluid_elem_info = elem_is_fluid ? *face->elemInfo() : *face->neighborInfo();
    const Real p_elem_value = _p->getElemValue(fluid_elem_info, time_arg);
    const Real matrix_contribution =
        _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
    const Real rhs_contribution = _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);
    const Real reconstructed_p_grad_flux =
        p_elem_value * matrix_contribution - rhs_contribution;

    const auto face_id = face->id();
    const Real hbya_source = libmesh_map_find(_HbyA_flux, face_id);
    const Real transient_source = libmesh_map_find(_transient_projection_flux, face_id);
    const Real capillary_hydrostatic_source =
        libmesh_map_find(_capillary_hydrostatic_flux, face_id);
    const Real bc_source_sum = hbya_source + capillary_hydrostatic_source;
    const Real source_sum = bc_source_sum + transient_source;
    const Real final_face_flux = _face_mass_flux[face_id];

    _console << "Sharp-interface boundary audit (" << label << "): audit_step="
             << audit_counter << ", face_id=" << face_id
             << ", centroid=" << face->faceCentroid()
             << ", normal=" << face->normal()
             << ", p_elem=" << p_elem_value
             << ", boundary_matrix=" << matrix_contribution
             << ", boundary_rhs=" << rhs_contribution
             << ", reconstructed_p_grad_flux=" << reconstructed_p_grad_flux
             << ", HbyA_source=" << hbya_source
             << ", transient_source=" << transient_source
             << ", capillary_hydrostatic_source=" << capillary_hydrostatic_source
             << ", bc_source_sum=" << bc_source_sum
             << ", full_source_sum=" << source_sum
             << ", final_face_flux=" << final_face_flux << std::endl;
  };

  for (const auto * elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Point centroid = elem_info->elem()->vertex_average();
    const Real liquid_metric = std::abs(centroid(1) - 0.475) + 0.1 * std::abs(centroid(0) - 0.25);
    if (liquid_metric < best_liquid_metric)
    {
      best_liquid_metric = liquid_metric;
      target_liquid_elem_info = elem_info;
    }

    const Real gas_metric = std::abs(centroid(1) - 0.525) + 0.1 * std::abs(centroid(0) - 0.25);
    if (gas_metric < best_gas_metric)
    {
      best_gas_metric = gas_metric;
      target_gas_elem_info = elem_info;
    }
  }

  auto audit_cell = [this, &time_arg](const ElemInfo * elem_info, const std::string & label)
  {
    if (!elem_info || _dim < 2)
      return;

    const auto * const elem = elem_info->elem();
    const auto elem_arg = makeElemArg(elem);
    const Real rho = _rho(elem_arg, time_arg);
    const RealVectorValue grad_p = MetaPhysicL::raw_value(_p->gradient(elem_arg, time_arg));
    const Real hydro_cell_accel_y = _hydrostatic_density_gradient_cell_acceleration
                                        ? (*_hydrostatic_density_gradient_cell_acceleration)(
                                              elem_arg, time_arg)(1)
                                        : 0.0;
    const Real surface_cell_accel_y =
        _surface_tension_cell_acceleration
            ? (*_surface_tension_cell_acceleration)(elem_arg, time_arg)(1)
            : 0.0;
    const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[1]][0];
    const Real ainv_y = (*_Ainv_raw[1])(dof);
    const Real hbya_y = (*_HbyA_raw[1])(dof);
    const Real reconstructed_delta_y =
        reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg)(1);
    const Real base_velocity_y = -hbya_y - ainv_y * grad_p(1);
    const Real face_reconstructed_velocity_y = -hbya_y + reconstructed_delta_y;
    const Real final_velocity_y =
        (*_momentum_implicit_systems[1]->current_local_solution)(dof);

    _console << "Sharp-interface cell audit (" << label << "): audit_step=" << audit_counter
             << ", elem_id=" << elem->id()
             << ", centroid=" << elem->vertex_average()
             << ", rho=" << rho
             << ", grad_p_y=" << grad_p(1)
             << ", HbyA_y=" << hbya_y
             << ", Ainv_y=" << ainv_y
             << ", hydro_cell_accel_y=" << hydro_cell_accel_y
             << ", surface_cell_accel_y=" << surface_cell_accel_y
             << ", reconstructed_delta_y=" << reconstructed_delta_y
             << ", base_velocity_y=" << base_velocity_y
             << ", face_reconstructed_velocity_y=" << face_reconstructed_velocity_y
             << ", final_velocity_y=" << final_velocity_y << std::endl;
  };

  audit_internal_face(target_face);
  audit_boundary_face(target_bottom_face, "bottom");
  audit_boundary_face(target_top_face, "top");
  audit_cell(target_liquid_elem_info, "liquid_near_interface");
  audit_cell(target_gas_elem_info, "gas_near_interface");
}

void
SharpInterfaceRhieChowMassFlux::auditRepresentativePredictorBodyForce() const
{
  if (!_dim)
    return;

  const auto time_arg = Moose::currentState();
  FaceVectorField face_field(_moose_mesh, blockIDs(), "audit_momentum_predictor_body_force_face");
  const bool have_face_field = populateMomentumPredictorBodyForceFaceField(face_field, time_arg);
  FaceVectorField pressure_face_field(
      _moose_mesh, blockIDs(), "audit_momentum_predictor_pressure_force_face");
  const bool have_pressure_face_field =
      const_cast<SharpInterfaceRhieChowMassFlux *>(this)->populateMomentumPredictorPressureForceFaceField(
          pressure_face_field, time_arg);

  const ElemInfo * target_liquid_elem_info = nullptr;
  const ElemInfo * target_gas_elem_info = nullptr;
  Real best_liquid_metric = std::numeric_limits<Real>::max();
  Real best_gas_metric = std::numeric_limits<Real>::max();

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Point centroid = elem_info->elem()->vertex_average();
    const Real liquid_metric = std::abs(centroid(1) - 0.475) + 0.1 * std::abs(centroid(0) - 0.25);
    if (liquid_metric < best_liquid_metric)
    {
      best_liquid_metric = liquid_metric;
      target_liquid_elem_info = elem_info;
    }

    const Real gas_metric = std::abs(centroid(1) - 0.525) + 0.1 * std::abs(centroid(0) - 0.25);
    if (gas_metric < best_gas_metric)
    {
      best_gas_metric = gas_metric;
      target_gas_elem_info = elem_info;
    }
  }

  const auto audit_cell = [this,
                           &time_arg,
                           &face_field,
                           &pressure_face_field,
                           have_face_field,
                           have_pressure_face_field](const ElemInfo * elem_info,
                                                     const std::string & label)
  {
    if (!elem_info)
      return;

    const auto * const elem = elem_info->elem();
    const auto elem_arg = makeElemArg(elem);
    const Real rho = _rho(elem_arg, time_arg);
    const RealVectorValue grad_p = MetaPhysicL::raw_value(_p->gradient(elem_arg, time_arg));
    const RealVectorValue legacy_pressure_force = -grad_p;
    const RealVectorValue face_pressure_force =
        have_pressure_face_field
            ? evaluateFaceBasedMomentumPredictorPressureForceDensity(
                  elem_info, time_arg, &pressure_face_field)
            : RealVectorValue();
    const RealVectorValue legacy_body_force =
        evaluateLegacyMomentumPredictorBodyForceDensity(elem_info, time_arg);
    const RealVectorValue face_body_force =
        have_face_field
            ? evaluateFaceBasedMomentumPredictorBodyForceDensity(elem_info, time_arg, &face_field)
            : RealVectorValue();
    const RealVectorValue face_field_reconstruction =
        have_face_field ? face_field(elem_arg, time_arg) : RealVectorValue();
    const RealVectorValue hydro_cell_accel =
        _hydrostatic_density_gradient_cell_acceleration
            ? MetaPhysicL::raw_value((*_hydrostatic_density_gradient_cell_acceleration)(elem_arg,
                                                                                        time_arg))
            : RealVectorValue();
    const RealVectorValue surface_cell_accel =
        _surface_tension_cell_acceleration
            ? MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg))
            : RealVectorValue();

    _console << "  Sharp predictor body-force audit (" << label << "): elem_id=" << elem->id()
             << ", centroid=" << elem->vertex_average()
             << ", rho=" << rho
             << ", -grad_p=" << (-grad_p)
             << ", legacy_pressure_force=" << legacy_pressure_force
             << ", face_pressure_force=" << face_pressure_force
             << ", legacy_body_force=" << legacy_body_force
             << ", face_body_force=" << face_body_force
             << ", raw_face_reconstruction=" << face_field_reconstruction
             << ", legacy_total_force=" << (legacy_pressure_force + legacy_body_force)
             << ", face_total_force=" << (face_pressure_force + face_body_force)
             << ", hydro_cell_accel=" << hydro_cell_accel
             << ", surface_cell_accel=" << surface_cell_accel << std::endl;
  };

  _console << "  Sharp predictor body-force audit: face_based_mode="
           << _use_face_based_predictor_body_force
           << ", have_face_field=" << have_face_field
           << ", have_pressure_face_field=" << have_pressure_face_field << std::endl;
  audit_cell(target_liquid_elem_info, "liquid_near_interface");
  audit_cell(target_gas_elem_info, "gas_near_interface");
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
SharpInterfaceRhieChowMassFlux::clearPressureCoupledVelocityCorrectionAudit()
{
  _pressure_coupled_velocity_correction_audit_valid = false;
  _last_pressure_coupled_velocity_delta_l2 = 0.0;
  _last_pressure_coupled_velocity_delta_max = 0.0;
  _last_pressure_coupled_velocity_worst_elem_id = 0;
  _last_pressure_coupled_velocity_worst_centroid = Point();
}

void
SharpInterfaceRhieChowMassFlux::printPressureCoupledVelocityCorrectionAudit(
    const std::string & label) const
{
  if (!_pressure_coupled_velocity_correction_audit_valid)
  {
    _console << "  Sharp pressure-coupled velocity correction audit (" << label
             << "): unavailable" << std::endl;
    return;
  }

  _console << "  Sharp pressure-coupled velocity correction audit (" << label
           << "): |deltaU|_2=" << _last_pressure_coupled_velocity_delta_l2
           << " max|deltaU|=" << _last_pressure_coupled_velocity_delta_max
           << " worst_elem_id=" << _last_pressure_coupled_velocity_worst_elem_id
           << " centroid=" << _last_pressure_coupled_velocity_worst_centroid << std::endl;
}

void
SharpInterfaceRhieChowMassFlux::applyAdditionalCellVelocityCorrection()
{
  // Mirror interFoam's pEqn.H more closely by reconstructing the cell-velocity
  // correction from the discrete phig - pEqn.flux term, rather than from the
  // full pressure predictor phiHbyA.

  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  Real delta_velocity_squared_sum = 0.0;
  Real max_delta_velocity = 0.0;
  bool found_delta_velocity = false;
  _last_pressure_coupled_velocity_worst_elem_id = 0;
  _last_pressure_coupled_velocity_worst_centroid = Point();

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto & solution = *(_momentum_implicit_systems[system_i]->solution);

    for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    {
      if (!hasBlocks(elem_info->subdomain_id()))
        continue;

      const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];
      const RealVectorValue delta_velocity =
          reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);

      if (system_i == 0)
      {
        Real delta_velocity_norm_sq = 0.0;
        for (const auto dim_i : make_range(_dim))
          delta_velocity_norm_sq += delta_velocity(dim_i) * delta_velocity(dim_i);

        const Real delta_velocity_norm = std::sqrt(delta_velocity_norm_sq);
        delta_velocity_squared_sum += delta_velocity_norm_sq;

        if (!found_delta_velocity || delta_velocity_norm > max_delta_velocity)
        {
          found_delta_velocity = true;
          max_delta_velocity = delta_velocity_norm;
          _last_pressure_coupled_velocity_worst_elem_id = elem_info->elem()->id();
          _last_pressure_coupled_velocity_worst_centroid = elem_info->elem()->vertex_average();
        }
      }

      solution.set(dof, -(*_HbyA_raw[system_i])(dof) + delta_velocity(system_i));
    }

    solution.close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }

  _last_pressure_coupled_velocity_delta_l2 = std::sqrt(delta_velocity_squared_sum);
  _last_pressure_coupled_velocity_delta_max = max_delta_velocity;
  _pressure_coupled_velocity_correction_audit_valid = found_delta_velocity;
}
