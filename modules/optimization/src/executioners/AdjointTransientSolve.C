//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointTransientSolve.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MooseUtils.h"

#include "libmesh/sparse_matrix.h"
#include "libmesh/numeric_vector.h"

using namespace libMesh;

InputParameters
AdjointTransientSolve::validParams()
{
  InputParameters params = AdjointSolve::validParams();
  return params;
}

AdjointTransientSolve::AdjointTransientSolve(Executioner & ex)
  : AdjointSolve(ex),
    Restartable(this, "Executioners"),
    _old_time_residual(_nl_adjoint.getResidualTimeVector()),
    _forward_solutions(declareRecoverableData<std::vector<std::string>>("forward_solutions"))
{
}

bool
AdjointTransientSolve::solve()
{
  bool converged = AdjointSolve::solve();

  // Gather the contribution of this timestep to add to the next solve's source
  if (converged)
    evaluateTimeResidual(_nl_adjoint.solution(), _old_time_residual);

  return converged;
}

void
AdjointTransientSolve::insertForwardSolution(int tstep)
{
  // Time step should not be negative
  if (tstep < 0)
    mooseError("Negative time step occurred.");
  auto t_step = cast_int<std::size_t>(tstep); // Avoid compiler warnings
  // Should not be inserting a time greater the last one inserted
  if (t_step > _forward_solutions.size())
    mooseError("Trying to insert a solution at a time-step greater than one past the  previously "
               "inserted time step. Previous time step = ",
               (int)_forward_solutions.size() - 1,
               ", inserted time step = ",
               t_step,
               ".");

  // Add vector name to member variable if this time has not occurred
  if (t_step == _forward_solutions.size())
    _forward_solutions.push_back(getForwardSolutionName(tstep));

  // Get/add vector from/to adjoint system
  auto & solution = _nl_adjoint.addVector(_forward_solutions[t_step], false, PARALLEL);

  // Set the vector to the inserted solution
  solution = _nl_forward.solution();
}

void
AdjointTransientSolve::setForwardSolution(int tstep)
{
  // Make sure the time step was saved
  if (tstep < 0 || tstep >= cast_int<int>(_forward_solutions.size()))
    mooseError("Could not find forward solution at time step ", tstep, ".");
  auto t_step = cast_int<std::size_t>(tstep); // Avoid compiler warnings

  // Copy the solutions to states that exist in the system
  unsigned int state = 0;
  while (_nl_forward.hasSolutionState(state))
  {
    const auto & name = t_step > state ? _forward_solutions[t_step - state] : _forward_solutions[0];
    _nl_forward.solutionState(state) = _nl_adjoint.getVector(name);
    state++;
  }

  _nl_forward.update();
}

void
AdjointTransientSolve::assembleAdjointSystem(SparseMatrix<Number> & matrix,
                                             const NumericVector<Number> & solution,
                                             NumericVector<Number> & rhs)
{
  // Assemble the steady-state version of the adjoint problem
  AdjointSolve::assembleAdjointSystem(matrix, solution, rhs);
  // Add the contribution from old solutions
  rhs += _old_time_residual;
}

void
AdjointTransientSolve::evaluateTimeResidual(const NumericVector<Number> & solution,
                                            NumericVector<Number> & residual)
{
  // This tag should exist, but the matrix might not necessarily be added
  auto time_matrix_tag = _problem.getMatrixTagID("TIME");
  // Use the adjoint system matrix to hold the time Jacobian
  auto & time_matrix = static_cast<ImplicitSystem &>(_nl_adjoint.system()).get_system_matrix();

  // Make sure we tell the problem which system we are evaluating
  _problem.setCurrentNonlinearSystem(_forward_sys_num);
  // Accumulate the time part of the Jacobian into the adjoint matrix
  _problem.computeJacobianTag(*_nl_forward.currentSolution(), time_matrix, time_matrix_tag);

  // Perform multiplication of the adjoint solution on the transpose of the time Jacobian
  residual.zero();
  residual.add_vector_transpose(solution, time_matrix);
}
