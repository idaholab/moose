//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdjointSolve.h"
#include "Restartable.h"

/**
 * Transient adjoint solve object. @see TransientAndAdjoint for example usage. General algorithm:
 *
 *   1. Set forward initial conditions
 *   2. Save the forward initial solution using insertForwardSolution(0)
 *   3. for t_step = 1,...,num_step
 *   4.   Solve forward time step
 *   5.   Save forward solution using insertForwardSolution(t_step)
 *   6. Set _old_time_residual = 0
 *   6. for t_step = num_step,...,1
 *   7.   Set forward solution using setForwardSolution(t_step)
 *   8.   Solve adjoint time step: A* u* = b* + _old_time_residual
 *   9.   Evaluate time residual: _old_time_residual = A_t* u*
 *
 * where A* is the transpose of the linearized forward operator at the specified timestep, u* is the
 * adjoint solution, and A_t* is the transpose of the linearized forward time operator.
 */
class AdjointTransientSolve : public AdjointSolve, public Restartable
{
public:
  AdjointTransientSolve(Executioner & ex);

  static InputParameters validParams();

  /**
   * Overriding parent class so the time-derivative residual is stored for the next time step
   */
  virtual bool solve() override;

  /**
   * This function should be called after every converged forward time step. It adds a new vector
   * (if necessary) to the adjoint system representing the forward solution at the given time step.
   *
   * @param tstep Time step of the forward solution in which to store in _forward_solutions
   */
  void insertForwardSolution(int tstep);

  /**
   * Takes the previously saved forward solutions residing in the adjoint system and copies them to
   * the available solution states in the forward systems. This should be called at each adjoint
   * time step.
   *
   * @param tstep The forward time step index being evaluated during the adjoint solve.
   */
  void setForwardSolution(int tstep);

protected:
  /**
   * Overriding parent class so the previous time-derivative residual is added to the
   * right-hand-side of the linear solve
   */
  virtual void assembleAdjointSystem(SparseMatrix<Number> & matrix,
                                     const NumericVector<Number> & solution,
                                     NumericVector<Number> & rhs) override;

  /**
   * This evaluates the time part of the adjoint residual. This is used to accumulate the source
   * contribution from previous adjoint time steps for the next adjoint time step. It works by
   * evaluating the Jacobian of the time-derivative term on the forward system and multiplying the
   * transpose by the current adjoint solution.
   *
   * @param solution Current adjoint solution
   * @param residual Vector to save the result into
   */
  void evaluateTimeResidual(const NumericVector<Number> & solution,
                            NumericVector<Number> & residual);

  /**
   * Prescribed name of the forward solution at a specified time step
   *
   * @param tstep The forward time step
   * @return std::string The name of the vector representing the forward solution on the adjoint
   * system
   */
  static std::string getForwardSolutionName(int tstep)
  {
    return "forward_solution" + std::to_string(tstep);
  }

private:
  /// The residual contribution from previous adjoint solutions
  NumericVector<Number> & _old_time_residual;
  /// Name of the forward solution at each time step residing in the adjoint system
  std::vector<std::string> & _forward_solutions;
};
