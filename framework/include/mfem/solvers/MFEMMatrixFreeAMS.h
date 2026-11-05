//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMLORLinearSolverBase.h"

/**
 * Wrapper for mfem::MatrixFreeAMS solver.
 */
class MFEMMatrixFreeAMS : public Moose::MFEM::LORLinearSolverBase<mfem::MatrixFreeAMS>
{
public:
  static InputParameters validParams();

  MFEMMatrixFreeAMS(const InputParameters &);

  void ConstructSolver() override;

protected:
  virtual void SetSolverParameters(mfem::MatrixFreeAMS &) override {}

private:
  // Constructs the mfem::MatrixFreeAMS solver for the given operator.
  void BuildSolver(const mfem::Operator & op);

  /**
   * Rebuilds the mfem::MatrixFreeAMS solver on every SetOperator call. mfem::MatrixFreeAMS keeps a
   * reference to the operator it was built with and its own SetOperator is a no-op, but within a
   * Newton solve the Krylov solver passes each new Jacobian to its preconditioner only through
   * mfem::Solver::SetOperator.
   */
  class Rebuilder : public mfem::Solver
  {
  public:
    explicit Rebuilder(MFEMMatrixFreeAMS & owner) : _owner(owner) {}
    void SetOperator(const mfem::Operator & op) override;
    void Mult(const mfem::Vector & x, mfem::Vector & y) const override;

  private:
    MFEMMatrixFreeAMS & _owner;
  };

  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const int _inner_pi_its;
  const int _inner_g_its;
  /// Smoother for mfem::MatrixFreeAMS: the user's "smoother", or _default_smoother
  mfem::Solver * _smoother = nullptr;
  /// Default smoother, used when "smoother" is not set
  std::unique_ptr<mfem::Solver> _default_smoother;
  /// Solver built by BuildSolver(); declared last so it is destroyed before the smoother it uses
  std::unique_ptr<mfem::MatrixFreeAMS> _matrix_free_ams;
};

#endif
