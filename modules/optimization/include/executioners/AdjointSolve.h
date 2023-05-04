//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

// Forward declarations
class NonlinearSystemBase;
namespace libMesh
{
template <typename T>
class SparseMatrix;
template <typename T>
class NumericVector;
}

/**
 * The solve object is responsible for solving the adjoint version of a forward model. It does this
 * by solving a linear system with a transposed matrix and a source. The matrix is evaluated from
 * the forward model's Jacobian, using the converged solution. The source is computed by evaluating
 * the residual of a secondary nonlinear-system representing the adjoint system, in which the
 * adjoint solution is 0.
 */
class AdjointSolve : public SolveObject
{
public:
  AdjointSolve(Executioner & ex);

  static InputParameters validParams();

  /**
   * Solve the adjoint system with the following procedure:
   *   1. Call the _inner_solve
   *   2. Execute user-objects, auxkernels, and multiapps on ADJOINT_TIMESTEP_BEGIN
   *   3. Assemble the adjoint system:
   *     3a. Evaluate forward system Jacobian
   *     3b. Evaluate adjoint system residual
   *   4. Solve adjoint system by calling libMesh::linearSolver::adjoint_solve
   *   5. Execute user-objects, auxkernels, and multiapps on ADJOINT_TIMESTEP_END
   *
   * @return true Inner solve, multiapps, and adjoint solve all converged
   * @return false Inner solve, multiapps, or adjoint solve did not converge
   */
  virtual bool solve() override;

protected:
  /**
   * Checks whether the forward and adjoint systems are consistent
   */
  void checkIntegrity();

  /**
   * Assembles adjoint system
   *
   * @param matrix Un-transposed matrix (will be transposed later in solver)
   * @param solution Adjoint solution (basically the initial guess for the solver)
   * @param rhs The adjoint source (i.e. -residual)
   */
  virtual void assembleAdjointSystem(SparseMatrix<Number> & matrix,
                                     const NumericVector<Number> & solution,
                                     NumericVector<Number> & rhs);

  /**
   * Helper function for applying nodal BCs to the adjoint matrix and RHS.
   * Say there is a BC setting the d-th DoF to a dirichlet condition on the forward problem.
   * This basically sets the d-th column of the matrix to zero,
   * the d-th entry of the matrix diagonal to one,
   * and the d-th entry of the RHS to the solution passed in.
   *
   * @param matrix The matrix whose columns are set to 0
   * @param solution The solution to replace the entries of the RHS
   * @param rhs  The RHS to to replace with the solution
   */
  void applyNodalBCs(SparseMatrix<Number> & matrix,
                     const NumericVector<Number> & solution,
                     NumericVector<Number> & rhs);

  /// The number of the nonlinear system representing the forward model
  const unsigned int _forward_sys_num;
  /// The number of the nonlinear system representing the adjoint model
  const unsigned int _adjoint_sys_num;
  /// The nonlinear system representing the forward model
  NonlinearSystemBase & _nl_forward;
  /// The nonlinear system representing the adjoint model
  NonlinearSystemBase & _nl_adjoint;
};
