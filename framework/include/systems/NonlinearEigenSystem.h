//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"

#include "NonlinearSystemBase.h"
#include "SlepcEigenSolverConfiguration.h"

#include "libmesh/eigen_system.h"

// forward declarations
class EigenProblem;
class KernelBase;
class ResidualObject;

#ifdef LIBMESH_HAVE_SLEPC

/**
 * Nonlinear eigenvalue system to be solved
 */
class NonlinearEigenSystem : public NonlinearSystemBase
{
public:
  NonlinearEigenSystem(EigenProblem & problem, const std::string & name);

  virtual void solve() override;

  /**
   * Quit the current solve as soon as possible.
   */
  virtual void stopSolve() override;

  /**
   * Returns the current nonlinear iteration number.  In libmesh, this is
   * updated during the nonlinear solve, so it should be up-to-date.
   */
  virtual unsigned int getCurrentNonlinearIterationNumber() override;

  virtual void setupFiniteDifferencedPreconditioner() override;

  /**
   * Returns the convergence state
   *
   * @return true if converged, otherwise false
   */
  virtual bool converged() override;

  virtual NumericVector<Number> & RHS() override;

  /*
   *  A residual vector at MOOSE side for AX
   */
  NumericVector<Number> & residualVectorAX();

  /*
   * A residual vector at MOOSE side for BX
   */
  NumericVector<Number> & residualVectorBX();

  void attachSLEPcCallbacks();

  /**
   * Get the number of converged eigenvalues
   *
   * @return The number of converged eigenvalues
   */
  unsigned int getNumConvergedEigenvalues() const { return _eigen_sys.get_n_converged(); };

  virtual NonlinearSolver<Number> * nonlinearSolver() override;

  /**
   * Retrieve snes from slepc eigen solver. It is valid for only nonlinear eigen solver.
   * You should see a big error if you do this for linear solver.
   */
  virtual SNES getSNES() override;

  /**
   * Retrieve EPS (SLEPc eigen solver)
   */
  virtual EPS getEPS();

  EigenSystem & sys() { return _eigen_sys; }

  /**
   * For eigenvalue problems (including standard and generalized), inhomogeneous (Dirichlet or
   * Neumann)
   * boundary conditions are not allowed.
   */
  void checkIntegrity();

  /**
   * Return the Nth converged eigenvalue.
   *
   * @return The Nth converged eigenvalue as a complex number, i.e. the first and the second number
   * is the real and the imaginary part of
   * the eigenvalue, respectively.
   */
  std::pair<Real, Real> getConvergedEigenvalue(dof_id_type n) const;

  /**
   * Return the Nth converged eigenvalue and copies the respective eigen vector to the solution
   * vector.
   *
   * @return The Nth converged eigenvalue as a complex number, i.e. the first and the second number
   * is the real and the imaginary part of
   * the eigenvalue, respectively.
   */
  std::pair<Real, Real> getConvergedEigenpair(dof_id_type n) const;

  /**
   * Get the number of converged eigenvalues
   *
   * @return all converged eigenvalues as complex numbers
   */
  const std::vector<std::pair<Real, Real>> & getAllConvergedEigenvalues() const
  {
    return _eigen_values;
  }

  /**
   * Vector tag ID of right hand side
   */
  TagID eigenVectorTag() const { return _Bx_tag; }

  /**
   * Vector tag ID of left hand side
   */
  TagID nonEigenVectorTag() const { return _Ax_tag; }

  /**
   * Matrix tag ID of right hand side
   */
  TagID eigenMatrixTag() const { return _B_tag; }

  /**
   * Matrix tag ID of left hand side
   */
  TagID nonEigenMatrixTag() const { return _A_tag; }

  std::set<TagID> defaultVectorTags() const override;
  std::set<TagID> defaultMatrixTags() const override;

  /**
   * If the preconditioning matrix includes eigen kernels
   */
  void precondMatrixIncludesEigenKernels(bool precond_matrix_includes_eigen)
  {
    _precond_matrix_includes_eigen = precond_matrix_includes_eigen;
  }

  bool precondMatrixIncludesEigenKernels() const { return _precond_matrix_includes_eigen; }

  TagID precondMatrixTag() const { return _precond_tag; }

  virtual void attachPreconditioner(Preconditioner<Number> * preconditioner) override;

  Preconditioner<Number> * preconditioner() const { return _preconditioner; }

  virtual void turnOffJacobian() override;

  void residualAndJacobianTogether() override;

protected:
  virtual void postAddResidualObject(ResidualObject & object) override;

  void computeScalingJacobian() override;
  void computeScalingResidual() override;

  EigenSystem & _eigen_sys;
  EigenProblem & _eigen_problem;
  std::unique_ptr<SlepcEigenSolverConfiguration> _solver_configuration;
  std::vector<std::pair<Real, Real>> _eigen_values;
  unsigned int _n_eigen_pairs_required;
  NumericVector<Number> & _work_rhs_vector_AX;
  NumericVector<Number> & _work_rhs_vector_BX;
  TagID _Ax_tag;
  TagID _Bx_tag;
  TagID _A_tag;
  TagID _B_tag;
  TagID _precond_tag;
  bool _precond_matrix_includes_eigen;
  // Libmesh preconditioner
  Preconditioner<Number> * _preconditioner;
};

#else

class NonlinearEigenSystem : public libMesh::ParallelObject
{
public:
  NonlinearEigenSystem(EigenProblem & problem, const std::string & name);

  /**
   * Returns the convergence state
   * @return true if converged, otherwise false
   */
  bool converged() { return false; }

  void checkIntegrity() {}
};

#endif
