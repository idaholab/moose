//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearSystemBase.h"
#include "ComputeResidualFunctor.h"
#include "ComputeFDResidualFunctor.h"
#include "ComputeResidualAndJacobian.h"
#include "SubProblem.h"
#include "MooseError.h"

#include "libmesh/transient_system.h"
#include "libmesh/nonlinear_implicit_system.h"

/**
 * Nonlinear system to be solved
 *
 * It is a part of FEProblemBase ;-)
 */
class NonlinearSystem : public NonlinearSystemBase
{
public:
  NonlinearSystem(FEProblemBase & problem, const std::string & name);
  virtual ~NonlinearSystem();

  virtual void solve() override;

  void init() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() override;

  /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  virtual unsigned int getCurrentNonlinearIterationNumber() override
  {
    return _nl_implicit_sys.get_current_nonlinear_iteration_number();
  }

  virtual void setupFiniteDifferencedPreconditioner() override;

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  virtual bool converged() override;

  virtual NumericVector<Number> & RHS() override { return *_nl_implicit_sys.rhs; }

  virtual NonlinearSolver<Number> * nonlinearSolver() override
  {
    return _nl_implicit_sys.nonlinear_solver.get();
  }

  virtual SNES getSNES() override;

  virtual NonlinearImplicitSystem & sys() { return _nl_implicit_sys; }

  virtual void attachPreconditioner(Preconditioner<Number> * preconditioner) override;

  void residualAndJacobianTogether() override;

protected:
  void computeScalingJacobian() override;
  void computeScalingResidual() override;

  NonlinearImplicitSystem & _nl_implicit_sys;
  ComputeResidualFunctor _nl_residual_functor;
  ComputeFDResidualFunctor _fd_residual_functor;
  ComputeResidualAndJacobian _resid_and_jac_functor;

private:
  /**
   * Form preconditioning matrix via a standard finite difference method
   * column-by-column. This method computes both diagonal and off-diagonal
   * entrices regardless of the structure pattern of the Jacobian matrix.
   */
  void setupStandardFiniteDifferencedPreconditioner();

  /**
   * According to the nonzero pattern provided in the matrix, a graph is constructed.
   * A coloring algorithm is applied to the graph. The graph is partitioned into several
   * independent subgraphs (colors), and a finte difference method is applied color-by-color
   * to form a preconditioning matrix. If the number of colors is small, this method is much
   * faster than the standard FD. But there is an issue. If the matrix provided by users does not
   * represent the actual structure of the true Jacobian, the matrix computed via coloring could
   * be wrong or inaccurate. In this case, users should switch to the standard finite difference
   * method.
   */
  void setupColoringFiniteDifferencedPreconditioner();

  bool _use_coloring_finite_difference;

  bool _solution_is_invalid;
};
