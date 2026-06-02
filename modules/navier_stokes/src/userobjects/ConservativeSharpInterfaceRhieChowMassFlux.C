//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

#include "LinearSystem.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

#include <algorithm>
#include <cmath>
#include <numeric>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceRhieChowMassFlux);

InputParameters
ConservativeSharpInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = ConservativeSharpInterfaceRhieChowMassFluxBase::validParams();
  params.set<MooseEnum>("hydrostatic_predictor_discretization") = "discrete_density_sn_grad";
  params.addParam<bool>(
      "use_face_based_reduced_pressure_predictor_contract",
      false,
      "Deprecated compatibility parameter. The reference-parity sharp-interface path now uses the "
      "single velocity-form HbyA contract.");
  params.addClassDescription(
      "Sharp-interface Rhie-Chow face-flux provider for the reference-parity velocity momentum "
      "path. The primary unknowns are U components; density weighting enters through the "
      "momentum operator and mass-flux functors.");
  return params;
}

ConservativeSharpInterfaceRhieChowMassFlux::ConservativeSharpInterfaceRhieChowMassFlux(
    const InputParameters & params)
  : ConservativeSharpInterfaceRhieChowMassFluxBase(params),
    _continuity_error(_fe_problem.mesh(), blockIDs(), "conservative_continuity_error", false)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    UserObject::_subproblem.addFunctor("conservative_continuity_error", _continuity_error, tid);
}

bool
ConservativeSharpInterfaceRhieChowMassFlux::debugUsingCachedPredictorOperator() const
{
  return canUseCachedMomentumPredictorOperator();
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugCurrentVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Velocity component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*_momentum_implicit_systems[component]->current_local_solution)(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPreVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  return debugCurrentVelocityComponent(elem_info, component);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPostVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  return debugCurrentVelocityComponent(elem_info, component);
}

Real
ConservativeSharpInterfaceRhieChowMassFlux::debugLastWritebackPressureDeltaVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Velocity component index out of range.");
  libmesh_ignore(elem_info);
  return 0.0;
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
  const NumericVector<Number> & solution = *(momentum_system->solution);

  auto solution_parallel = solution.zero_clone();
  *solution_parallel = solution_override;
  solution_parallel->close();

  auto diagonal_parallel = solution.zero_clone();
  diagonal_parallel->close();
  mmat->get_diagonal(*diagonal_parallel);
  diagonal_parallel->close();
  diagonal_raw = *diagonal_parallel;
  diagonal_raw.close();

  auto working_vector = solution.zero_clone();
  working_vector->close();
  auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
  mooseAssert(working_vector_petsc,
              "The vectors used in ConservativeSharpInterfaceRhieChowMassFlux need to be convertible to PetscVectors.");

  auto base_parallel = solution.zero_clone();
  base_parallel->close();
  mmat->vector_mult(*base_parallel, *solution_parallel);
  base_parallel->close();
  working_vector_petsc->pointwise_mult(*diagonal_parallel, *solution_parallel);
  working_vector_petsc->close();
  base_parallel->add(-1.0, *working_vector_petsc);
  base_parallel->close();
  base_parallel->add(-1.0, rhs);
  base_parallel->close();
  base_raw = *base_parallel;
  base_raw.close();
}

void
ConservativeSharpInterfaceRhieChowMassFlux::buildVelocityPredictorState(
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

  auto & pressure_gradient =
      const_cast<ConservativeSharpInterfaceRhieChowMassFlux *>(this)->selectPressureGradient(
          with_updated_pressure);
  const bool split_predictor_operator = splitMomentumPredictorOperator();

  if (!split_predictor_operator)
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

  *working_vector_petsc = 1.0;
  ainv_raw.pointwise_divide(*working_vector_petsc, ainv_raw);
  hbya_raw.pointwise_mult(hbya_raw, ainv_raw);

  if (_pressure_projection_method == "consistent")
  {
    auto row_sum = current_local_solution.zero_clone();
    row_sum->zero();
    const auto local_size = mmat->local_m();
    for (const auto row_i : make_range(local_size))
    {
      const auto global_index = mmat->row_start() + row_i;
      std::vector<numeric_index_type> indices;
      std::vector<Real> values;
      mmat->get_row(global_index, indices, values);
      row_sum->add(global_index, std::accumulate(values.cbegin(), values.cend(), 0.0));
    }
    row_sum->close();

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
ConservativeSharpInterfaceRhieChowMassFlux::debugVelocityPredictorBaseRawComponent(
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
ConservativeSharpInterfaceRhieChowMassFlux::debugVelocityPredictorHbyAComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _momentum_implicit_systems.size(), "Momentum component index out of range.");

  const auto & current_local_solution = *(_momentum_implicit_systems[component]->current_local_solution);
  auto hbya_raw = current_local_solution.zero_clone();
  auto ainv_raw = current_local_solution.zero_clone();
  computePredictorOperatorBaseForSolution(component, current_local_solution, *hbya_raw, *ainv_raw);
  buildVelocityPredictorState(component, false, *hbya_raw, *ainv_raw);

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return (*hbya_raw)(dof);
}

void
ConservativeSharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size(),
              "Momentum component index out of range.");

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    const Real force_density =
        reducedPressureMomentumPredictorForceDensity(*elem_info, Moose::currentState())(system_i);
    rhs.add(dof_indices[0], force_density * elem_info->volume() * elem_info->coordFactor());
  }
  rhs.close();
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
