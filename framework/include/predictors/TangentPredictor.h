//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Predictor.h"

#include <memory>

namespace libMesh
{
template <typename T>
class LinearSolver;
template <typename T>
class SparseMatrix;
}

/**
 * Predicts the next solution from the accepted tangent response to the previous load increment.
 */
class TangentPredictor : public Predictor
{
public:
  static InputParameters validParams();

  TangentPredictor(const InputParameters & parameters);
  virtual ~TangentPredictor();

  virtual void timestepAccepted() override;
  virtual bool shouldApply() override;
  virtual void apply(NumericVector<Number> & sln) override;

protected:
  /// Build and initialize the linear solver used for the accepted-step tangent solve.
  void setupLinearSolver();

  /// Assemble the tagged load residual at the supplied time and accepted solution.
  void computeLoadResidualAtTime(Real time, NumericVector<Number> & residual);

  /// Return whether the next predictor application is known to be skipped.
  bool skipNextStepAfterAcceptedTime(Real accepted_time) const;

  /// Return whether the last predictor linear solve converged.
  bool linearSolveConverged() const;

  /// Compute the accepted-step direction using the full tangent matrix.
  bool computeFullTangentDirection(SparseMatrix<Number> & jacobian);

  /// Compute the accepted-step direction with the Jacobi approximation diag(K_T)^{-1} Delta F.
  bool computeDiagonalTangentDirection(SparseMatrix<Number> & jacobian);

  const TagName _load_vector_tag_name;
  TagID _load_vector_tag;
  const std::string _linear_solver_options_prefix;
  const bool _use_diagonal_approximation;
  const bool _error_on_solve_failure;
  const Real _linear_solve_tol;
  const unsigned int _linear_solve_max_its;

  NumericVector<Number> & _load_scratch;
  NumericVector<Number> & _load_increment;
  NumericVector<Number> * const _diagonal;
  NumericVector<Number> & _direction;

  std::unique_ptr<libMesh::LinearSolver<Number>> _linear_solver;

  bool & _state_valid;
  Real & _accepted_dt;
};
