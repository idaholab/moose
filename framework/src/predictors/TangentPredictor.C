//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentPredictor.h"
#include "FEProblemBase.h"
#include "MooseUtils.h"
#include "NonlinearSystemBase.h"
#include "PetscSupport.h"
#include "ScopedVectorTagAssociation.h"

#include "libmesh/linear_solver.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

#include <cmath>

registerMooseObject("MooseApp", TangentPredictor);

InputParameters
TangentPredictor::validParams()
{
  InputParameters params = Predictor::validParams();
  params.addClassDescription("Predicts the next solution from the accepted tangent response to a "
                             "tagged external load increment.");
  params.addRequiredParam<TagName>(
      "load_vector_tag",
      "Residual vector tag containing the external load terms used to compute the accepted load "
      "increment.");
  params.addParam<std::string>(
      "linear_solver_options_prefix",
      "tangent_predictor_",
      "PETSc/libMesh options prefix for the predictor's auxiliary linear solve.");
  params.addParam<Real>("linear_solve_tol",
                        "Relative tolerance for the predictor's auxiliary tangent solve. If not "
                        "supplied, the executioner's linear tolerance is used.");
  params.addParam<unsigned int>(
      "linear_solve_max_its",
      "Maximum number of linear iterations for the predictor's auxiliary tangent solve. If not "
      "supplied, the executioner's linear maximum iteration count is used.");
  params.addParam<bool>(
      "use_diagonal_approximation",
      false,
      "Whether to approximate the tangent response with diag(K_T)^{-1} times the tagged load "
      "increment instead of solving with the full tangent matrix.");
  params.addParam<bool>(
      "error_on_solve_failure",
      true,
      "Whether to error if the auxiliary tangent solve or diagonal approximation fails. If false, "
      "the predictor warns and skips prediction until a later accepted step records a valid "
      "tangent direction.");
  return params;
}

TangentPredictor::TangentPredictor(const InputParameters & parameters)
  : Predictor(parameters),
    _load_vector_tag_name(getParam<TagName>("load_vector_tag")),
    _load_vector_tag(Moose::INVALID_TAG_ID),
    _linear_solver_options_prefix(getParam<std::string>("linear_solver_options_prefix")),
    _use_diagonal_approximation(getParam<bool>("use_diagonal_approximation")),
    _error_on_solve_failure(getParam<bool>("error_on_solve_failure")),
    _linear_solve_tol(parameters.isParamValid("linear_solve_tol")
                          ? getParam<Real>("linear_solve_tol")
                          : _fe_problem.es().parameters.get<Real>("linear solver tolerance")),
    _linear_solve_max_its(
        parameters.isParamValid("linear_solve_max_its")
            ? getParam<unsigned int>("linear_solve_max_its")
            : _fe_problem.es().parameters.get<unsigned int>("linear solver maximum iterations")),
    _load_scratch(_nl.addVector(name() + "_load_scratch", false, libMesh::PARALLEL)),
    _load_increment(_nl.addVector(name() + "_load_increment", false, libMesh::PARALLEL)),
    _diagonal(_use_diagonal_approximation
                  ? &_nl.addVector(name() + "_diagonal", false, libMesh::PARALLEL)
                  : nullptr),
    _direction(_nl.addVector(name() + "_direction", true, libMesh::PARALLEL)),
    // These restartable flags tell a restarted run whether the checkpointed direction vector is
    // ready to use and what accepted step size produced it.
    _state_valid(declareRestartableData<bool>("state_valid", false)),
    _accepted_dt(declareRestartableData<Real>("accepted_dt", 0))
{
  if (!_fe_problem.vectorTagExists(_load_vector_tag_name))
  {
    if (_fe_problem.matrixTagExists(_load_vector_tag_name))
      paramError("load_vector_tag",
                 "The load vector tag '",
                 _load_vector_tag_name,
                 "' was declared as a matrix tag. Declare it in [Problem] with "
                 "extra_tag_vectors so it is an extra residual vector tag.");

    paramError("load_vector_tag",
               "The load vector tag '",
               _load_vector_tag_name,
               "' does not exist. Declare it in [Problem] with extra_tag_vectors.");
  }

  _load_vector_tag = _fe_problem.getVectorTagID(_load_vector_tag_name);

  if (_fe_problem.vectorTagType(_load_vector_tag) != Moose::VECTOR_TAG_RESIDUAL)
    paramError("load_vector_tag",
               "The load vector tag must be a residual vector tag. The supplied tag '",
               _load_vector_tag_name,
               "' is not a residual tag.");

  if (!_nl.hasVector(_load_vector_tag))
    paramError("load_vector_tag",
               "The load vector tag '",
               _load_vector_tag_name,
               "' is not associated with nonlinear system '",
               _nl.name(),
               "'. Declare it in [Problem] with extra_tag_vectors.");

  if (_linear_solve_tol <= 0)
    paramError("linear_solve_tol", "The predictor linear solve tolerance must be positive.");

  if (_linear_solve_max_its == 0)
    paramError("linear_solve_max_its",
               "The predictor linear solve maximum iteration count must be greater than zero.");
}

TangentPredictor::~TangentPredictor() = default;

void
TangentPredictor::setupLinearSolver()
{
  if (!_linear_solver)
  {
    _linear_solver = libMesh::LinearSolver<Number>::build(_nl.comm());
    _linear_solver->init(
        _linear_solver_options_prefix.empty() ? nullptr : _linear_solver_options_prefix.c_str());
    Moose::PetscSupport::setLinearSolverDefaults(_fe_problem, *_linear_solver);
  }

  // libMesh documents init_systems() as required after a System reinit. Keep it here so the
  // predictor remains valid after mesh or system changes between accepted timesteps.
  _linear_solver->init_systems(_nl.system());
}

// This helper intentionally associates the load tag with the same residual vector twice. The first
// association lets computeResidualTag() fill the vector with load-tagged residual objects.
// computeResidualTag() clears that association before returning, so the second association is
// required for the nodal-BC-with-load-tag corner case: load-tagged nodal BC rows must be written
// into the same vector before the problem's persistent extra tag vector is restored.
void
TangentPredictor::computeLoadResidualAtTime(Real time, NumericVector<Number> & residual)
{
  const auto saved_time = _fe_problem.time();

  _fe_problem.time() = time;
  _fe_problem.setCurrentNonlinearSystem(_nl.number());

  ScopedVectorTagAssociation load_tag(_nl, _load_vector_tag);

  // Assemble only the residual objects that opt in to the load tag. Nodal BC rows are then
  // overwritten with the constraint residual so the predictor RHS is compatible with Dirichlet
  // rows in the accepted tangent matrix.
  load_tag.associate(residual);
  _fe_problem.computeResidualTag(_solution_predictor, residual, _load_vector_tag);
  // computeResidualTag() temporarily associates residual with the load tag and then clears that
  // association. Re-associate the same vector while applying nodal BCs so load-tagged nodal rows
  // land in this residual too, then restore the problem's persistent extra tag vector.
  load_tag.associate(residual);
  _nl.applyNodalBCsResidual(residual);
  load_tag.restore();
  residual.close();

  _fe_problem.time() = saved_time;
}

bool
TangentPredictor::skipNextStepAfterAcceptedTime(Real accepted_time) const
{
  if (_scale == 0)
    return true;

  // At the next predictor application this accepted time will be timeOld(). If the base predictor
  // is configured to skip that old time, there is no need to assemble and store a direction now.
  for (const auto skip_time_old : _skip_times_old)
    if (MooseUtils::absoluteFuzzyEqual(accepted_time, skip_time_old, _timestep_tolerance))
      return true;

  return false;
}

bool
TangentPredictor::linearSolveConverged() const
{
  mooseAssert(_linear_solver, "The tangent predictor linear solver has not been created");
  return _linear_solver->get_converged_reason() > 0;
}

bool
TangentPredictor::computeFullTangentDirection(SparseMatrix<Number> & jacobian)
{
  _direction.zero();
  _direction.close();

  // This computes the action of K_T^{-1} on the load increment without forming an inverse.
  setupLinearSolver();
  const auto solve_result = _linear_solver->solve(
      jacobian, _direction, _load_increment, _linear_solve_tol, _linear_solve_max_its);
  _direction.close();

  if (!linearSolveConverged())
  {
    if (_error_on_solve_failure)
      mooseError("TangentPredictor auxiliary linear solve failed after ",
                 solve_result.first,
                 " iterations with final residual ",
                 solve_result.second,
                 ".");

    mooseWarning(
        "TangentPredictor auxiliary linear solve failed; skipping prediction until a valid "
        "accepted-step direction is available.");
    return false;
  }

  return true;
}

bool
TangentPredictor::computeDiagonalTangentDirection(SparseMatrix<Number> & jacobian)
{
  mooseAssert(_diagonal, "The tangent diagonal vector should be allocated in diagonal mode");

  jacobian.get_diagonal(*_diagonal);
  _diagonal->close();

  bool zero_loaded_diagonal = false;
  for (auto i = _diagonal->first_local_index(); i < _diagonal->last_local_index(); ++i)
    if (std::abs((*_diagonal)(i)) == 0.0 && std::abs(_load_increment(i)) != 0.0)
    {
      zero_loaded_diagonal = true;
      break;
    }
  _nl.comm().max(zero_loaded_diagonal);

  if (zero_loaded_diagonal)
  {
    if (_error_on_solve_failure)
      mooseError("TangentPredictor diagonal approximation encountered a nonzero load increment on "
                 "a zero tangent diagonal entry, which would produce a non-finite direction.");

    mooseWarning("TangentPredictor diagonal approximation encountered a nonzero load increment on "
                 "a zero tangent diagonal entry; skipping prediction until a valid accepted-step "
                 "direction is available.");
    return false;
  }

  // This is the Jacobi approximation to the tangent solve. The matrix is still assembled so the
  // diagonal includes the same residual objects as the full tangent, but no KSP solve is needed.
  // The vector operations remain parallel because each owned entry only needs the matching
  // diagonal and load-increment entry.
  _diagonal->reciprocal();
  _direction.pointwise_mult(*_diagonal, _load_increment);
  _direction.close();

  if (!std::isfinite(_direction.linfty_norm()))
  {
    if (_error_on_solve_failure)
      mooseError("TangentPredictor diagonal approximation produced a non-finite direction. This "
                 "usually indicates a zero diagonal entry in the accepted tangent matrix.");

    mooseWarning(
        "TangentPredictor diagonal approximation produced a non-finite direction; skipping "
        "prediction until a valid accepted-step direction is available.");
    return false;
  }

  return true;
}

void
TangentPredictor::timestepAccepted()
{
  if (_dt <= 0 || skipNextStepAfterAcceptedTime(_fe_problem.time()))
  {
    _state_valid = false;
    return;
  }

  const auto accepted_time = _fe_problem.time();
  const auto old_time = _fe_problem.timeOld();

  _nl.solution().localize(_solution_predictor);

  // BodyForce contributes R_load(t) = -f(t) test. Therefore old - new gives the positive external
  // load increment for an increasing load at the fixed accepted solution.
  computeLoadResidualAtTime(old_time, _load_increment);
  computeLoadResidualAtTime(accepted_time, _load_scratch);
  _load_increment.add(-1.0, _load_scratch);
  _load_increment.close();

  auto & lm_sys = dynamic_cast<libMesh::NonlinearImplicitSystem &>(_nl.system());
  auto & jacobian = *lm_sys.matrix;
  // Assemble the tangent at the accepted solution so the full and diagonal modes use the same
  // matrix state for the stored predictor direction.
  _fe_problem.computeJacobian(_solution_predictor, jacobian, _nl.number());
  jacobian.close();

  if (!(_use_diagonal_approximation ? computeDiagonalTangentDirection(jacobian)
                                    : computeFullTangentDirection(jacobian)))
  {
    _state_valid = false;
    return;
  }

  _accepted_dt = _dt;
  _state_valid = true;
}

bool
TangentPredictor::shouldApply()
{
  if (!Predictor::shouldApply())
    return false;

  if (!_state_valid || _scale == 0 || _dt <= 0 || _accepted_dt <= 0)
    return false;

  return true;
}

void
TangentPredictor::apply(NumericVector<Number> & sln)
{
  _console << "  Applying tangent predictor with scale factor = " << _scale << std::endl;

  const Real dt_adjusted_scale_factor = _scale * _dt / _accepted_dt;
  if (dt_adjusted_scale_factor != 0.0)
  {
    sln.add(dt_adjusted_scale_factor, _direction);
    sln.close();
  }
}
