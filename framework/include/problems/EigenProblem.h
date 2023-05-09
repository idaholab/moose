//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE Includes
#include "FEProblemBase.h"
#include "Eigenvalue.h"

class NonlinearEigenSystem;

/**
 * Problem for solving eigenvalue problems
 */
class EigenProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  EigenProblem(const InputParameters & parameters);

  virtual std::string solverTypeString() override
  {
    return Moose::stringify(solverParams()._eigen_solve_type);
  }

#ifdef LIBMESH_HAVE_SLEPC
  virtual void solve(unsigned int nl_sys_num = 0) override;

  virtual void init() override;

  virtual bool nlConverged(unsigned int nl_sys_num) override;

  unsigned int getNEigenPairsRequired() const { return _n_eigen_pairs_required; }
  void setNEigenPairsRequired(unsigned int n_eigen_pairs)
  {
    _n_eigen_pairs_required = n_eigen_pairs;
  }
  bool isGeneralizedEigenvalueProblem() const { return _generalized_eigenvalue_problem; }
  bool isNonlinearEigenvalueSolver() const;
  // silences warning in debug mode about the other computeJacobian signature being hidden
  using FEProblemBase::computeJacobian;

  NonlinearEigenSystem & getNonlinearEigenSystem(unsigned int nl_sys_num = 0);
  NonlinearEigenSystem & getCurrentNonlinearEigenSystem();

  virtual void checkProblemIntegrity() override;

  /**
   * A flag indicates if a negative sign is used in eigen kernels.
   * If the negative sign is used, eigen kernels are consistent in nonlinear solver.
   * In nonlinear solver, RHS kernels always have a negative sign.
   */
  bool negativeSignEigenKernel() const { return _negative_sign_eigen_kernel; }

  /**
   * Set postprocessor and normalization factor
   * 'Postprocessor' is often used to compute an integral of physics variables
   */
  void setNormalization(const PostprocessorName & pp,
                        const Real value = std::numeric_limits<Real>::max());

  /**
   * Set an initial eigenvalue for initial normalization
   */
  void setInitialEigenvalue(const Real initial_eigenvalue)
  {
    _initial_eigenvalue = initial_eigenvalue;
  }

  /**
   * Whether or not we are doing free power iteration. It is used in convergence check.
   * We need to mark the solver as "converged" when doing free power to retrieve
   * the final solution from SLPEc
   */
  bool doFreePowerIteration() const { return _do_free_power_iteration; }

  /**
   * Set a flag to indicate whether or not we are doing free power iterations.
   */
  void doFreePowerIteration(bool do_power) { _do_free_power_iteration = do_power; }

  /**
   * Eigenvector need to be scaled back if it was scaled in an earlier stage
   * Scaling eigen vector does not affect solution (eigenvalue, eigenvector),
   * but it does affect the convergence rate. To have an optimal convergence rate,
   * We pre-scale eigen vector using the same factor as the one computed in
   * "postScaleEigenVector"
   */
  void preScaleEigenVector(const std::pair<Real, Real> & eig);

  /**
   * Normalize eigen vector. Scale eigen vector such as ||x|| = _normal_factor
   * This might be useful when coupling to other physics
   */
  void postScaleEigenVector();

  /**
   * Scale eigenvector. Scaling_factor is often computed based on physics.
   */
  void scaleEigenvector(const Real scaling_factor);

  /**
   * Set eigen problem type. Don't need to use this if we use Newton eigenvalue solver.
   */
  void setEigenproblemType(Moose::EigenProblemType eigen_problem_type);

  /**
   * Compute the residual of Ax - \lambda Bx. If there is no \lambda yet, "1" will
   * be used.
   */
  virtual Real computeResidualL2Norm() override;

  /**
   * Form a Jacobian matrix for all kernels and BCs with a given tag
   */
  virtual void computeJacobianTag(const NumericVector<Number> & soln,
                                  SparseMatrix<Number> & jacobian,
                                  TagID tag) override;

  /**
   * Form several matrices simultaneously
   */
  void computeMatricesTags(const NumericVector<Number> & soln,
                           const std::vector<std::unique_ptr<SparseMatrix<Number>>> & jacobians,
                           const std::set<TagID> & tags);

  /**
   * Form two Jacobian matrices, where each is associated with one tag, through one
   * element-loop.
   */
  void computeJacobianAB(const NumericVector<Number> & soln,
                         SparseMatrix<Number> & jacobianA,
                         SparseMatrix<Number> & jacobianB,
                         TagID tagA,
                         TagID tagB);

  virtual void computeJacobianBlocks(std::vector<JacobianBlock *> & blocks,
                                     unsigned int nl_sys_num = 0) override;

  /**
   * Form a vector for all kernels and BCs with a given tag
   */
  virtual void computeResidualTag(const NumericVector<Number> & soln,
                                  NumericVector<Number> & residual,
                                  TagID tag) override;

  /**
   * Form two vetors, where each is associated with one tag, through one
   * element-loop.
   */
  void computeResidualAB(const NumericVector<Number> & soln,
                         NumericVector<Number> & residualA,
                         NumericVector<Number> & residualB,
                         TagID tagA,
                         TagID tagB);

  /**
   * Convenience function for performing execution of MOOSE systems.
   * We override this function to perform an initial scaling.
   */
  virtual void execute(const ExecFlagType & exec_type) override;

  /**
   * For nonlinear eigen solver, a good initial value can help convergence.
   * Should set initial values for only eigen variables.
   */
  void initEigenvector(const Real initial_value);

  /**
   * Which eigenvalue is active
   */
  unsigned int activeEigenvalueIndex() const { return _active_eigen_index; }

  /**
   * Hook up monitors for SNES and KSP
   */
  virtual void initPetscOutput() override;

  /**
   * Whether or not to output eigenvalue inverse. The inverse is useful for
   * neutronics community
   */
  bool outputInverseEigenvalue() const { return _output_inverse_eigenvalue; }

  /**
   * Set a flag to indicate whether or not to output eigenvalue inverse.
   */
  void outputInverseEigenvalue(bool inverse) { _output_inverse_eigenvalue = inverse; }

  /**
   * Whether or not we are in a linear solver iteration
   */
  bool onLinearSolver() const { return _on_linear_solver; }

  /**
   * Set a flag to indicate whether or not we are in a linear solver iteration
   */
  void onLinearSolver(bool ols) { _on_linear_solver = ols; }

  /**
   * Whether or not matrices are constant
   */
  bool constantMatrices() const { return _constant_matrices; }

  /**
   * Set a flag to indicate whether or not we use constant matrices
   */
  void constantMatrices(bool cm) { _constant_matrices = cm; }

  /**
   * Whether or not constant matrices were already formed
   */
  bool wereMatricesFormed() const { return _matrices_formed; }

  /**
   * Set a flag to indicate whether or not constant matrices were already formed
   */
  void wereMatricesFormed(bool mf) { _matrices_formed = mf; }

private:
  /**
   * Do some free/extra power iterations
   */
  void doFreeNonlinearPowerIterations(unsigned int free_power_iterations);

  /**
   * Adjust eigen vector by either scaling the existing values or setting new values
   * The operations are applied for only eigen variables
   */
  void adjustEigenVector(const Real value, bool scaling);

#endif

  using FEProblemBase::_nl;

protected:
  unsigned int _n_eigen_pairs_required;
  bool _generalized_eigenvalue_problem;
  std::shared_ptr<NonlinearEigenSystem> _nl_eigen;

  /// Whether or not use negative sign for Bx. Use negative sign by default to
  /// make the eigen system consistent with nonlinear system
  bool _negative_sign_eigen_kernel;
  /// Which eigenvalue is used to compute residual. By default the zeroth eigenvalue
  /// is used.
  unsigned int _active_eigen_index;

  /// Whether or not we are doing free power iteration. Free power iteration is
  /// often used to compute initial guess for Newton eigen solver. It is automatically
  /// triggered by Eigenvalue Executioner
  bool _do_free_power_iteration;
  /// Whether or not output eigenvalue as its inverse. By default, we output regular eigenvalue.
  bool _output_inverse_eigenvalue;
  /// Whether or not we are in linear solver
  bool _on_linear_solver;
  /// Whether or not matrices had been formed
  bool _matrices_formed;
  /// Whether or not require constant matrices
  bool _constant_matrices;
  /// Whether or not we normalize eigenvector
  bool _has_normalization;
  /// Postprocessor used to compute a factor from eigenvector
  PostprocessorName _normalization;
  /// Postprocessor target value. The value of postprocessor should equal to
  /// '_normal_factor' by adjusting eigenvector
  Real _normal_factor;
  /// A flag to indicate if it is the first time calling the solve
  bool & _first_solve;
  /// A value used for initial normalization
  Real _initial_eigenvalue;
};

#ifdef LIBMESH_HAVE_SLEPC

inline NonlinearEigenSystem &
EigenProblem::getNonlinearEigenSystem(const unsigned int nl_sys_num)
{
  if (nl_sys_num > 0)
    mooseError("eigen problems do not currently support multiple nonlinear eigen systems");
  return *_nl_eigen;
}

inline NonlinearEigenSystem &
EigenProblem::getCurrentNonlinearEigenSystem()
{
  return *_nl_eigen;
}

#endif
