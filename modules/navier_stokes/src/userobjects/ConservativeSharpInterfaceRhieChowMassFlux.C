//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

#include "FVUtils.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearSystem.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/petsc_matrix.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceRhieChowMassFlux);

InputParameters
ConservativeSharpInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = ConservativeSharpInterfaceRhieChowMassFluxBase::validParams();
  params.set<MooseEnum>("hydrostatic_predictor_discretization") = "discrete_density_sn_grad";
  params.addParam<bool>(
      "use_face_based_reduced_pressure_predictor_contract",
      false,
      "Whether the conservative predictor exports HbyA using the face-based reduced-pressure "
      "force contract rather than the legacy cell pressure/body-force split.");
  params.addClassDescription(
      "Sharp-interface Rhie-Chow face-flux provider for true conservative rho*u momentum "
      "systems. This runs in parallel to the existing velocity-form sharp-interface "
      "implementation and uses the discrete face-normal reduced-pressure hydrostatic "
      "predictor operator by default.");
  return params;
}

ConservativeSharpInterfaceRhieChowMassFlux::ConservativeSharpInterfaceRhieChowMassFlux(
    const InputParameters & params)
  : ConservativeSharpInterfaceRhieChowMassFluxBase(params),
    _use_face_based_reduced_pressure_predictor_contract(
        getParam<bool>("use_face_based_reduced_pressure_predictor_contract")),
    _continuity_error(_fe_problem.mesh(), blockIDs(), "conservative_continuity_error", false)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    UserObject::_subproblem.addFunctor("conservative_continuity_error", _continuity_error, tid);
}

bool
ConservativeSharpInterfaceRhieChowMassFlux::debugUsingCachedPredictorOperator() const
{
  // The conservative path now rebuilds the predictor from the derived U view so
  // the raw rhoU-form cache cannot silently contaminate HbyA.
  return false;
}

void
ConservativeSharpInterfaceRhieChowMassFlux::clearAuthoritativeVelocitySolution()
{
  _authoritative_velocity_solution_raw.clear();
  _authoritative_velocity_solution_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFlux::rebuildAuthoritativeVelocitySolutionFromMomentum()
{
  clearAuthoritativeVelocitySolution();
  _authoritative_velocity_solution_raw.reserve(_momentum_implicit_systems.size());

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    mooseAssert(momentum_system && momentum_system->current_local_solution,
                "The requested momentum component is not linked to "
                "ConservativeSharpInterfaceRhieChowMassFlux.");

    const auto & current_local_solution = *(momentum_system->current_local_solution);
    auto derived_velocity = current_local_solution.zero_clone();

    for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    {
      if (!hasBlocks(elem_info->subdomain_id()))
        continue;

      const auto & dof_indices =
          elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
      if (dof_indices.empty())
        continue;

      const auto dof = dof_indices[0];
      const Real rho = _rho(makeElemArg(elem_info->elem()), Moose::currentState());
      const Real rho_u = current_local_solution(dof);
      derived_velocity->set(dof, std::abs(rho) > libMesh::TOLERANCE ? rho_u / rho : 0.0);
    }

    derived_velocity->close();
    _authoritative_velocity_solution_raw.push_back(std::move(derived_velocity));
  }

  _authoritative_velocity_solution_valid =
      _authoritative_velocity_solution_raw.size() == _momentum_implicit_systems.size() &&
      std::all_of(_authoritative_velocity_solution_raw.begin(),
                  _authoritative_velocity_solution_raw.end(),
                  [](const auto & vec) { return static_cast<bool>(vec); });
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugCurrentMomentumComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*_momentum_implicit_systems[component]->current_local_solution)(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugCurrentVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  if (_authoritative_velocity_solution_valid &&
      _authoritative_velocity_solution_raw.size() == _momentum_implicit_systems.size() &&
      _authoritative_velocity_solution_raw[component])
    return (*_authoritative_velocity_solution_raw[component])(dof);

  const Real rho_u = debugCurrentMomentumComponent(elem_info, component);
  const Real rho = _rho(makeElemArg(elem_info.elem()), Moose::currentState());
  return std::abs(rho) > libMesh::TOLERANCE ? rho_u / rho : 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPreMomentumComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  if (_last_writeback_pre_momentum.size() != _momentum_implicit_systems.size())
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  if (const auto it = _last_writeback_pre_momentum[component].find(dof);
      it != _last_writeback_pre_momentum[component].end())
    return it->second;

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPostMomentumComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  if (_last_writeback_post_momentum.size() != _momentum_implicit_systems.size())
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  if (const auto it = _last_writeback_post_momentum[component].find(dof);
      it != _last_writeback_post_momentum[component].end())
    return it->second;

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPressureDeltaVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  if (_last_writeback_pressure_delta_velocity.size() != _momentum_implicit_systems.size())
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  if (const auto it = _last_writeback_pressure_delta_velocity[component].find(dof);
      it != _last_writeback_pressure_delta_velocity[component].end())
    return it->second;

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPressureDeltaMomentumComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  const Real rho = _rho(makeElemArg(elem_info.elem()), Moose::currentState());
  return rho * debugLastWritebackPressureDeltaVelocityComponent(elem_info, component);
}

void
ConservativeSharpInterfaceRhieChowMassFlux::computePredictorOperatorBaseForSolution(
    const unsigned int system_i,
    const NumericVector<Number> & solution_override,
    NumericVector<Number> & base_raw,
    NumericVector<Number> & diagonal_raw) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");

  auto * momentum_system = _momentum_implicit_systems[system_i];
  auto * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
  mooseAssert(mmat,
              "The matrices used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscMatrix.");

  const NumericVector<Number> & rhs = *(momentum_system->rhs);
  mmat->get_diagonal(diagonal_raw);

  auto working_vector = solution_override.zero_clone();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscVectors.");

  mmat->vector_mult(base_raw, solution_override);
  working_vector_petsc->pointwise_mult(diagonal_raw, solution_override);
  base_raw.add(-1.0, *working_vector_petsc);
  base_raw.add(-1.0, rhs);
}

std::unique_ptr<NumericVector<Number>>
ConservativeSharpInterfaceRhieChowMassFlux::buildDerivedVelocitySolution(
    const unsigned int system_i) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");

  if (_authoritative_velocity_solution_valid &&
      _authoritative_velocity_solution_raw.size() == _momentum_implicit_systems.size() &&
      _authoritative_velocity_solution_raw[system_i])
    return _authoritative_velocity_solution_raw[system_i]->clone();

  const auto & current_local_solution = *(_momentum_implicit_systems[system_i]->current_local_solution);
  auto derived_velocity = current_local_solution.clone();
  *derived_velocity = current_local_solution;

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    const Real rho = _rho(makeElemArg(elem_info->elem()), Moose::currentState());
    const Real rho_u = current_local_solution(dof_indices[0]);
    derived_velocity->set(
        dof_indices[0], std::abs(rho) > libMesh::TOLERANCE ? rho_u / rho : 0.0);
  }

  derived_velocity->close();
  return derived_velocity;
}

void
ConservativeSharpInterfaceRhieChowMassFlux::buildVelocityProjectionDensityVector(
    const unsigned int system_i, NumericVector<Number> & density_raw) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");

  auto * density_vector = dynamic_cast<PetscVector<Number> *>(&density_raw);
  mooseAssert(density_vector,
              "The vectors used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscVectors.");

  *density_vector = 1.0;

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    const Real rho = _rho(makeElemArg(elem_info->elem()), Moose::currentState());
    density_raw.set(dof_indices[0], std::max(std::abs(rho), libMesh::TOLERANCE));
  }

  density_raw.close();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::convertConservativePredictorStateToVelocityForm(
    const unsigned int system_i,
    const bool with_updated_pressure,
    NumericVector<Number> & hbya_raw,
    NumericVector<Number> & ainv_raw) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");

  auto * momentum_system = _momentum_implicit_systems[system_i];
  const NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
  auto * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
  mooseAssert(mmat,
              "The matrices used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscMatrix.");

  auto working_vector = current_local_solution.zero_clone();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscVectors.");

  auto velocity_projection_density = current_local_solution.zero_clone();
  buildVelocityProjectionDensityVector(system_i, *velocity_projection_density);

  auto & pressure_gradient =
      const_cast<ConservativeSharpInterfaceRhieChowMassFlux *>(this)->selectPressureGradient(
          with_updated_pressure);
  const bool split_predictor_operator = splitMomentumPredictorOperator();

  if (!split_predictor_operator)
  {
    if (_use_face_based_reduced_pressure_predictor_contract)
    {
      auto face_based_predictor_force_rhs = current_local_solution.zero_clone();
      for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
        if (hasBlocks(elem_info->subdomain_id()))
        {
          const auto elem_dof =
              elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];
          const Real cell_volume = elem_info->volume() * elem_info->coordFactor();
          const Real rhs_contribution =
              reducedPressureMomentumPredictorForceDensity(*elem_info, Moose::currentState())(
                  system_i) *
              cell_volume;
          face_based_predictor_force_rhs->set(elem_dof, rhs_contribution);
        }
      face_based_predictor_force_rhs->close();
      hbya_raw.add(1.0, *face_based_predictor_force_rhs);
    }
    else
    {
      working_vector_petsc->pointwise_mult(*pressure_gradient[system_i], *_cell_volumes);
      hbya_raw.add(-1.0, *working_vector_petsc);

      if (!_body_force_kernels.empty())
      {
        auto explicit_body_force_rhs = current_local_solution.zero_clone();
        for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
          if (hasBlocks(elem_info->subdomain_id()))
          {
            const auto elem_dof =
                elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];

            Real rhs_contribution = 0.0;
            for (const auto & force_kernel : _body_force_kernels[system_i])
            {
              force_kernel->setCurrentElemInfo(elem_info);
              rhs_contribution += force_kernel->computeRightHandSideContribution();
            }

            explicit_body_force_rhs->set(elem_dof, rhs_contribution);
          }

        explicit_body_force_rhs->close();
        hbya_raw.add(1.0, *explicit_body_force_rhs);
      }
    }
  }

  // The conservative systems solve for q = rho*U. To export the pressure-side
  // predictor contract used by interFoam, transform the predictor operator from
  // q-space to U-space:
  //   A_q q = H,  q = D_rho U  =>  A_U U = H with A_U = A_q D_rho.
  // The off-diagonal/base term can therefore be formed with the live q field,
  // while the diagonal and consistent-projection row-sum must be scaled by rho.
  ainv_raw.pointwise_mult(ainv_raw, *velocity_projection_density);

  *working_vector_petsc = 1.0;
  ainv_raw.pointwise_divide(*working_vector_petsc, ainv_raw);
  hbya_raw.pointwise_mult(hbya_raw, ainv_raw);

  if (_pressure_projection_method == "consistent")
  {
    auto row_sum = current_local_solution.zero_clone();
    mmat->vector_mult(*row_sum, *velocity_projection_density);

    auto Ainv_full = current_local_solution.zero_clone();
    *working_vector_petsc = 1.0;
    Ainv_full->pointwise_divide(*working_vector_petsc, *row_sum);
    const auto Ainv_full_old = Ainv_full->clone();

    Ainv_full->add(-1.0, ainv_raw);
    working_vector_petsc->pointwise_mult(*Ainv_full, *pressure_gradient[system_i]);
    working_vector_petsc->pointwise_mult(*working_vector_petsc, *_cell_volumes);
    hbya_raw.add(-1.0, *working_vector_petsc);

    ainv_raw = *Ainv_full_old;
  }

  ainv_raw.pointwise_mult(ainv_raw, *_cell_volumes);
}

void
ConservativeSharpInterfaceRhieChowMassFlux::buildDerivedVelocityPredictorState(
    const unsigned int system_i,
    const bool with_updated_pressure,
    NumericVector<Number> & hbya_raw,
    NumericVector<Number> & ainv_raw) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");

  auto * momentum_system = _momentum_implicit_systems[system_i];
  const NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
  computePredictorOperatorBaseForSolution(system_i, current_local_solution, hbya_raw, ainv_raw);
  convertConservativePredictorStateToVelocityForm(
      system_i, with_updated_pressure, hbya_raw, ainv_raw);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLivePredictorBaseRawComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto & current_local_solution = *(_momentum_implicit_systems[component]->current_local_solution);
  auto base_raw = current_local_solution.zero_clone();
  auto diagonal_raw = current_local_solution.zero_clone();
  computePredictorOperatorBaseForSolution(component, current_local_solution, *base_raw, *diagonal_raw);

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*base_raw)(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugCachedPredictorBaseRawComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  if (_cached_predictor_operator_base_raw.size() != _momentum_implicit_systems.size() ||
      !_cached_predictor_operator_base_raw[component])
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*_cached_predictor_operator_base_raw[component])(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugDerivedVelocityPredictorBaseRawComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto & current_local_solution = *(_momentum_implicit_systems[component]->current_local_solution);
  auto base_raw = current_local_solution.zero_clone();
  auto diagonal_raw = current_local_solution.zero_clone();
  computePredictorOperatorBaseForSolution(component, current_local_solution, *base_raw, *diagonal_raw);

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*base_raw)(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugDerivedVelocityPredictorHbyAComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto & current_local_solution = *(_momentum_implicit_systems[component]->current_local_solution);
  auto hbya_raw = current_local_solution.zero_clone();
  auto ainv_raw = current_local_solution.zero_clone();
  buildDerivedVelocityPredictorState(component, false, *hbya_raw, *ainv_raw);

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*hbya_raw)(dof);
}

void
ConservativeSharpInterfaceRhieChowMassFlux::initFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  for (auto & fi : flowFaceInfo())
  {
    RealVectorValue face_momentum;

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      for (const auto dim_i : index_range(_vel))
        interpolate(InterpMethod::Average,
                    face_momentum(dim_i),
                    _vel[dim_i]->getElemValue(elem_info, time_arg),
                    _vel[dim_i]->getElemValue(neighbor_info, time_arg),
                    *fi,
                    true);
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

      for (const auto dim_i : index_range(_vel))
        face_momentum(dim_i) =
            boundary_normal_multiplier * boundaryMomentumComponentValue(fi, dim_i, time_arg);
    }

    if (!_pressure_equation_flux_valid)
      _face_mass_flux[fi->id()] = face_momentum * fi->normal();
  }
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::boundaryMomentumComponentValue(
    const FaceInfo * fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi, "FaceInfo should not be null when evaluating a boundary momentum value.");
  mooseAssert(component < _vel.size(), "Momentum component index out of range.");
  mooseAssert(!_vel[component]->isInternalFace(*fi),
              "boundaryMomentumComponentValue should only be called on boundary faces.");

  if (_velocity_boundary_state_valid && time_arg.state == Moose::currentState().state)
    if (const auto it = _boundary_velocity_face_values[component].find(fi->id());
        it != _boundary_velocity_face_values[component].end())
      return it->second;

  if (!fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1,
                "Expected at most one physical boundary id on a FV boundary face.");

    if (auto * bc_pointer = _vel[component]->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      bc_pointer->setupFaceData(
          fi,
          fi->faceType(
              std::make_pair(_vel[component]->number(), _vel[component]->sys().number())));
      return bc_pointer->computeBoundaryValue();
    }
  }

  const ElemInfo & elem_info =
      hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
  return _vel[component]->getElemValue(elem_info, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::boundaryVelocityComponentValue(
    const FaceInfo * fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  const Real rho_u = boundaryMomentumComponentValue(fi, component, time_arg);

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
  const Real face_rho = _rho(boundary_face, time_arg);

  return std::abs(face_rho) > libMesh::TOLERANCE ? rho_u / face_rho : 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::boundaryConservativeMassFluxTarget(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryConservativeMassFluxTarget should only be called on boundary faces.");

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  RealVectorValue face_momentum;
  for (const auto component : index_range(_vel))
    face_momentum(component) =
        boundary_normal_multiplier * boundaryMomentumComponentValue(fi, component, time_arg);

  return face_momentum * fi->normal();
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::pressureBoundaryTargetFlux(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  return boundaryConservativeVolumetricFluxTarget(fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::facePhysicalVelocityComponent(
    const FaceInfo * fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi, "FaceInfo should not be null when evaluating a conservative face velocity.");
  mooseAssert(component < _vel.size(), "Velocity component index out of range.");

  if (_vel[component]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
    const auto neighbor_dof =
        neighbor_info.dofIndices()[_global_momentum_system_numbers[component]][0];

    const Real elem_velocity =
        _authoritative_velocity_solution_valid &&
                _authoritative_velocity_solution_raw.size() == _momentum_implicit_systems.size() &&
                _authoritative_velocity_solution_raw[component]
            ? (*_authoritative_velocity_solution_raw[component])(elem_dof)
            : ([&]()
               {
                 const Real elem_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
                 return std::abs(elem_rho) > libMesh::TOLERANCE
                            ? _vel[component]->getElemValue(elem_info, time_arg) / elem_rho
                            : 0.0;
               })();
    const Real neighbor_velocity =
        _authoritative_velocity_solution_valid &&
                _authoritative_velocity_solution_raw.size() == _momentum_implicit_systems.size() &&
                _authoritative_velocity_solution_raw[component]
            ? (*_authoritative_velocity_solution_raw[component])(neighbor_dof)
            : ([&]()
               {
                 const Real neighbor_rho = _rho(makeElemArg(neighbor_info.elem()), time_arg);
                 return std::abs(neighbor_rho) > libMesh::TOLERANCE
                            ? _vel[component]->getElemValue(neighbor_info, time_arg) / neighbor_rho
                            : 0.0;
               })();
    return 0.5 * (elem_velocity + neighbor_velocity);
  }

  return boundaryVelocityComponentValue(fi, component, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::boundaryConservativeVolumetricFluxTarget(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryConservativeVolumetricFluxTarget should only be called on boundary faces.");

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  RealVectorValue face_velocity;
  for (const auto component : index_range(_vel))
    face_velocity(component) =
        boundary_normal_multiplier * boundaryVelocityComponentValue(fi, component, time_arg);

  return face_velocity * fi->normal();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors(
    const bool with_updated_pressure, const bool verbose)
{
  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;

  const auto time_arg = Moose::currentState();

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  std::vector<PetscVectorReader> pressure_gradient_readers;
  buildSelectedPressureGradientReaders(with_updated_pressure, pressure_gradient_readers);

  std::vector<PetscVectorReader> hbya_readers;
  hbya_readers.reserve(_HbyA_raw.size());
  for (const auto & hbya_raw : _HbyA_raw)
    hbya_readers.emplace_back(*hbya_raw);

  for (const auto * fi : _sharp_interface_face_info)
  {
    RealVectorValue face_hbya;

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();
      if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      {
        for (const auto dim_i : index_range(_vel))
        {
          const auto owner_dof =
              owner_info->dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          face_hbya(dim_i) = -hbya_readers[dim_i](owner_dof);
        }
      }
      else
      {
        for (const auto dim_i : index_range(_vel))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          const auto neighbor_dof =
              neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];

          interpolate(Moose::FV::InterpMethod::Average,
                      face_hbya(dim_i),
                      -hbya_readers[dim_i](elem_dof),
                      -hbya_readers[dim_i](neighbor_dof),
                      *fi,
                      true);
        }
      }
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

      const bool use_constrained_boundary_state = useConstrainedBoundaryPredictorState(fi);

      if (use_constrained_boundary_state)
      {
        for (const auto dim_i : make_range(_dim))
        {
          const Real boundary_value = boundaryVelocityComponentValue(fi, dim_i, time_arg);
          face_hbya(dim_i) =
              std::isfinite(boundary_value)
                  ? -boundary_value
                  : -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
          face_hbya(dim_i) *= boundary_normal_multiplier;
        }
      }
      else
      {
        for (const auto dim_i : index_range(_vel))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          face_hbya(dim_i) = -boundary_normal_multiplier * hbya_readers[dim_i](elem_dof);
        }
      }
    }

    const Real predictor_operator_volumetric_flux = face_hbya * fi->normal();
    const auto state =
        buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers, &pressure_gradient_readers);
    const RealVectorValue pressure_face_raw_ainv =
        interpolatePressureFaceRawAinv(fi, raw_ainv_readers);
    const Real normal_pressure_ainv =
        computeFaceNormalRawAinv(pressure_face_raw_ainv, state.face_normal);
    Real transient_projection_flux = 0.0;
    Real surface_tension_flux = 0.0;
    Real hydrostatic_flux = 0.0;

    if (_add_transient_projection_flux && !_suppress_startup_pressure_predictor_flux_sources)
    {
      if (_transient_projection_face_acceleration)
      {
        const RealVectorValue transient_acceleration =
            evaluateBoundaryAwareVectorFunctor(_transient_projection_face_acceleration, fi, time_arg);
        if (transient_acceleration != RealVectorValue())
        {
          const Real transient_mass_flux_density = projectPhysicalMassFluxDensity(
              state.face_rho, state.face_raw_ainv, transient_acceleration, state.face_normal);
          transient_projection_flux =
              -massFluxDensityToVolumetricNormalFlux(fi, transient_mass_flux_density);
        }
      }
      else
        transient_projection_flux =
            -computeDefaultTransientProjectionVolumetricFlux(fi, time_arg, state);
    }

    _transient_projection_flux[fi->id()] = transient_projection_flux;

    if (_add_capillary_hydrostatic_flux && !_suppress_startup_pressure_predictor_flux_sources)
    {
      RealVectorValue surface_acceleration;
      if (_surface_tension_face_acceleration)
        surface_acceleration =
            evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
      else if (_surface_tension_cell_acceleration)
        surface_acceleration =
            interpolateCellVectorFunctorToFace(_surface_tension_cell_acceleration, fi, time_arg);

      if (surface_acceleration != RealVectorValue())
      {
        const Real surface_mass_flux_density = projectPhysicalMassFluxDensity(
            state.face_rho, state.face_raw_ainv, surface_acceleration, state.face_normal);
        surface_tension_flux =
            massFluxDensityToVolumetricNormalFlux(fi, surface_mass_flux_density);
      }

      if (!_suppress_explicit_hydrostatic_pressure_flux)
      {
        const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
        const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
        hydrostatic_flux = -ghf * sn_grad_rho * normal_pressure_ainv;
      }
    }

    const Real phig_flux = surface_tension_flux + hydrostatic_flux;
    const Real pressure_predictor_base_flux =
        predictor_operator_volumetric_flux + transient_projection_flux;

    _debug_update_hydrostatic_branch_taken[fi->id()] = std::abs(phig_flux) > libMesh::TOLERANCE;
    _debug_update_hydrostatic_face_mass_flux_density_raw[fi->id()] = hydrostatic_flux;
    _debug_update_physical_capillary_hydrostatic_flux[fi->id()] = phig_flux;
    _capillary_hydrostatic_flux[fi->id()] = phig_flux;
    _phig_flux[fi->id()] = phig_flux;
    _pressure_Ainv[fi->id()] = pressure_face_raw_ainv;
    _pressure_predictor_base_flux[fi->id()] = pressure_predictor_base_flux;
    _phiHbyA_flux[fi->id()] = pressure_predictor_base_flux + phig_flux;
    _pressure_predictor_flux[fi->id()] = pressure_predictor_base_flux + phig_flux;
    _pressure_predictor_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(fi, _pressure_predictor_flux[fi->id()], time_arg);

    if (verbose)
    {
      _console << "Conservative sharp-interface predictor on face " << fi->id()
               << ": HbyA_flux=" << predictor_operator_volumetric_flux
               << ", transient_source_flux=" << _transient_projection_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
               << ", pressure_predictor_base_flux=" << _pressure_predictor_base_flux[fi->id()]
               << ", phiHbyA_flux=" << _phiHbyA_flux[fi->id()] << std::endl;
    }
  }

  _pressure_predictor_face_state_valid = true;
}

void
ConservativeSharpInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
{
  const auto time_arg = Moose::currentState();

  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();

  for (const auto * fi : flowFaceInfo())
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    RealVectorValue face_momentum;

    for (const auto component : index_range(_vel))
    {
      _boundary_velocity_face_values[component][fi->id()] =
          boundaryMomentumComponentValue(fi, component, time_arg);
      face_momentum(component) =
          boundary_normal_multiplier * _boundary_velocity_face_values[component][fi->id()];
    }

    if (!_pressure_equation_flux_valid)
      _face_mass_flux[fi->id()] = face_momentum * fi->normal();
  }

  _velocity_boundary_state_valid = true;
  cacheCurrentCorrectedVolumetricFlux();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::writeProvisionalMomentumToMomentumSolution(
    const Moose::StateArg & time_arg)
{
  std::vector<std::unique_ptr<NumericVector<Number>>> provisional_solution;
  provisional_solution.reserve(_momentum_implicit_systems.size());

  if (_last_writeback_pre_momentum.size() != _momentum_implicit_systems.size())
    _last_writeback_pre_momentum.resize(_momentum_implicit_systems.size());
  if (_last_writeback_post_momentum.size() != _momentum_implicit_systems.size())
    _last_writeback_post_momentum.resize(_momentum_implicit_systems.size());
  if (_last_writeback_pressure_delta_velocity.size() != _momentum_implicit_systems.size())
    _last_writeback_pressure_delta_velocity.resize(_momentum_implicit_systems.size());

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    _last_writeback_pre_momentum[system_i].clear();
    _last_writeback_post_momentum[system_i].clear();
    _last_writeback_pressure_delta_velocity[system_i].clear();
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    mooseAssert(
        momentum_system && momentum_system->current_local_solution,
        "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFlux.");
    provisional_solution.push_back(momentum_system->current_local_solution->zero_clone());
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const RealVectorValue pressure_delta =
        reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);
    const Real cell_rho = _rho(makeElemArg(elem_info->elem()), time_arg);

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      const auto & dof_indices =
          elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
      if (dof_indices.empty())
        continue;

      const auto dof = dof_indices[0];
      const Real pre_momentum =
          (*_momentum_implicit_systems[system_i]->current_local_solution)(dof);
      const Real delta_velocity = pressure_delta(system_i);
      const Real provisional_velocity = predictorVelocityComponent(*elem_info, system_i);
      const Real post_velocity = provisional_velocity + delta_velocity;
      const Real post_momentum = cell_rho * post_velocity;

      _last_writeback_pre_momentum[system_i][dof] = pre_momentum;
      _last_writeback_pressure_delta_velocity[system_i][dof] = delta_velocity;
      _last_writeback_post_momentum[system_i][dof] = post_momentum;

      provisional_solution[system_i]->set(dof, post_momentum);
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    provisional_solution[system_i]->close();
    *(_momentum_implicit_systems[system_i]->solution) = *provisional_solution[system_i];
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }

  rebuildAuthoritativeVelocitySolutionFromMomentum();
  cacheCurrentCorrectedVolumetricFlux();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::computeCellVelocity()
{
  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);
  writeProvisionalMomentumToMomentumSolution(time_arg);
  _velocity_boundary_state_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFlux::updateContinuityErrorField()
{
  _continuity_error.clear();

  const Real dt = _fe_problem.dt();
  const bool use_time_derivative = std::abs(dt) > libMesh::TOLERANCE;

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto elem_arg = makeElemArg(elem_info->elem());
    Real cont_err = 0.0;
    if (use_time_derivative)
      cont_err += (_rho(elem_arg, Moose::currentState()) - _rho(elem_arg, Moose::oldState())) / dt;

    _continuity_error[elem_info->elem()->id()] = cont_err;
  }

  for (const auto * fi : flowFaceInfo())
  {
    const auto flux_it = _face_mass_flux.find(fi->id());
    if (flux_it == _face_mass_flux.end())
      continue;

    const Real rho_phi = flux_it->second;
    const Real face_measure = fi->faceArea() * fi->faceCoord();

    if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
    {
      const auto & elem_info = *fi->elemInfo();
      const Real cell_volume = elem_info.volume() * elem_info.coordFactor();
      if (cell_volume > libMesh::TOLERANCE)
        _continuity_error[elem_info.elem()->id()] += rho_phi * face_measure / cell_volume;
    }

    if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
    {
      const auto & neighbor_info = *fi->neighborInfo();
      const Real cell_volume = neighbor_info.volume() * neighbor_info.coordFactor();
      if (cell_volume > libMesh::TOLERANCE)
        _continuity_error[neighbor_info.elem()->id()] -= rho_phi * face_measure / cell_volume;
    }
  }
}

void
ConservativeSharpInterfaceRhieChowMassFlux::populateConservativeCouplingFunctors(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_Ainv)
{
  using namespace Moose::FV;
  const auto time_arg = Moose::currentState();

  std::vector<PetscVectorReader> hbya_reader;
  hbya_reader.reserve(raw_hbya.size());
  for (const auto dim_i : index_range(raw_hbya))
    hbya_reader.emplace_back(*raw_hbya[dim_i]);

  std::vector<PetscVectorReader> ainv_reader;
  ainv_reader.reserve(raw_Ainv.size());
  for (const auto dim_i : index_range(raw_Ainv))
    ainv_reader.emplace_back(*raw_Ainv[dim_i]);

  for (auto & fi : flowFaceInfo())
  {
    Real face_rho = 0.0;
    RealVectorValue face_hbya;
    RealVectorValue density_times_face_hbya;
    auto & Ainv = _Ainv[fi->id()];
    auto & pressure_Ainv = _pressure_Ainv[fi->id()];

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();
      const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
      const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
      if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      {
        const bool owner_is_elem = owner_info->elem() == fi->elemPtr();
        const Real owner_rho = owner_is_elem ? elem_rho : neighbor_rho;
        face_rho = owner_rho;

        for (const auto dim_i : index_range(raw_hbya))
        {
          const auto owner_dof =
              owner_info->dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          face_hbya(dim_i) = -hbya_reader[dim_i](owner_dof);
          density_times_face_hbya(dim_i) = face_rho * face_hbya(dim_i);
          Ainv(dim_i) = face_rho * ainv_reader[dim_i](owner_dof);
        }
        pressure_Ainv = interpolatePressureFaceRawAinv(fi, ainv_reader);
      }
      else
      {
        interpolate(
            InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);

        for (const auto dim_i : index_range(raw_hbya))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          const auto neighbor_dof =
              neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];

          interpolate(InterpMethod::Average,
                      face_hbya(dim_i),
                      -hbya_reader[dim_i](elem_dof),
                      -hbya_reader[dim_i](neighbor_dof),
                      *fi,
                      true);
          interpolate(InterpMethod::Average,
                      density_times_face_hbya(dim_i),
                      -elem_rho * hbya_reader[dim_i](elem_dof),
                      -neighbor_rho * hbya_reader[dim_i](neighbor_dof),
                      *fi,
                      true);
          interpolate(InterpMethod::Average,
                      Ainv(dim_i),
                      elem_rho * ainv_reader[dim_i](elem_dof),
                      neighbor_rho * ainv_reader[dim_i](neighbor_dof),
                      *fi,
                      true);
          interpolate(InterpMethod::Average,
                      pressure_Ainv(dim_i),
                      ainv_reader[dim_i](elem_dof),
                      ainv_reader[dim_i](neighbor_dof),
                      *fi,
                      true);
        }
      }
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

      const bool use_constrained_boundary_state = useConstrainedBoundaryPredictorState(fi);

      if (use_constrained_boundary_state)
      {
        face_rho = _rho(boundary_face, Moose::currentState());
        for (const auto dim_i : make_range(_dim))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          const Real boundary_value = boundaryVelocityComponentValue(fi, dim_i, time_arg);
          face_hbya(dim_i) =
              std::isfinite(boundary_value)
                  ? -boundary_value
                  : -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
          face_hbya(dim_i) *= boundary_normal_multiplier;
          density_times_face_hbya(dim_i) = face_rho * face_hbya(dim_i);
          Ainv(dim_i) = face_rho * ainv_reader[dim_i](elem_dof);
          pressure_Ainv(dim_i) = ainv_reader[dim_i](elem_dof);
        }
      }
      else
      {
        face_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
        for (const auto dim_i : index_range(raw_hbya))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          face_hbya(dim_i) = -boundary_normal_multiplier * hbya_reader[dim_i](elem_dof);
          density_times_face_hbya(dim_i) = face_rho * face_hbya(dim_i);
          Ainv(dim_i) = face_rho * ainv_reader[dim_i](elem_dof);
          pressure_Ainv(dim_i) = ainv_reader[dim_i](elem_dof);
        }
      }
    }

    _HbyA_flux[fi->id()] = density_times_face_hbya * fi->normal();
  }
}

void
ConservativeSharpInterfaceRhieChowMassFlux::computeConservativeHbyA(const bool with_updated_pressure,
                                                                    const bool verbose)
{
  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "Computing conservative HbyA" << std::endl;
    _console << "************************************" << std::endl;
  }

  mooseAssert(_momentum_implicit_systems.size() && _momentum_implicit_systems[0],
              "The momentum system shall be linked before calling this function!");

  _pressure_equation_flux_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  _pressure_predictor_face_state_valid = false;
  _pressure_coupled_velocity_correction_valid = false;
  // The pressure corrector's phi/U pair should stay authoritative until the
  // next predictor rebuild actually starts. Clear the conservative predictor
  // cache here, at the beginning of that rebuild, instead of immediately after
  // writeback where boundary refreshes and diagnostics still need the solved
  // pressure state.
  clearMomentumPredictorOperatorCache();
  _HbyA_raw.clear();
  _Ainv_raw.clear();
  clearAuthoritativeVelocitySolution();

  const bool use_cached_predictor_operator = canUseCachedMomentumPredictorOperator();

  if (verbose && use_cached_predictor_operator)
    _console << "Using cached assembled conservative momentum predictor operator for HbyA/Ainv."
             << std::endl;

  for (auto system_i : index_range(_momentum_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    NumericVector<Number> & rhs = *(momentum_system->rhs);
    NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);

    if (use_cached_predictor_operator)
    {
      _Ainv_raw.push_back(_cached_predictor_diagonal_raw[system_i]->clone());
      _HbyA_raw.push_back(_cached_predictor_operator_base_raw[system_i]->clone());
    }
    else
    {
      _Ainv_raw.push_back(current_local_solution.zero_clone());
      _HbyA_raw.push_back(current_local_solution.zero_clone());
      computePredictorOperatorBase(system_i,
                                   *(_HbyA_raw.back()),
                                   *(_Ainv_raw.back()),
                                   nullptr);
    }

    NumericVector<Number> & Ainv = *(_Ainv_raw.back());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    convertConservativePredictorStateToVelocityForm(
        system_i, with_updated_pressure, HbyA, Ainv);

    if (verbose)
    {
      _console << "Conservative predictor base exported in velocity form component " << system_i
               << std::endl;
      rhs.print();
      HbyA.print();
      _console << "Conservative raw 1/A exported in velocity form component " << system_i
               << std::endl;
      Ainv.print();
    }
  }

  populateConservativeCouplingFunctors(_HbyA_raw, _Ainv_raw);

  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "DONE Computing conservative HbyA" << std::endl;
    _console << "************************************" << std::endl;
  }
}
