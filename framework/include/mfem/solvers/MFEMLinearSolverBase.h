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

#include "MFEMSolverBase.h"

class MFEMProblemSolve;

namespace Moose::MFEM
{
/**
 * Base class for linear MFEM solvers and preconditioners.
 */
class LinearSolverBase : public SolverBase
{
public:
  static InputParameters validParams();

  LinearSolverBase(const InputParameters & parameters);

  /// Retrieves the preconditioner userobject if present, sets the member pointer to
  /// said object if still unset, and sets the solver to use this preconditioner.
  template <typename T>
  void setPreconditioner(T & solver);

  /// Returns this solver's preconditioner
  LinearSolverBase * getPreconditioner() { return _preconditioner.get(); };

  /// Rebuild any Low-Order-Refined components from the unreduced bilinear form. Called only when
  /// isLOR() is true, before the assembled operator is available. Default no-op; override in
  /// solvers or preconditioners that construct their internals from the bilinear form.
  virtual void setupLOR(mfem::ParBilinearForm & /*a*/, mfem::Array<int> & /*tdofs*/) {}

  /// Updates the solver at the operator level. Default implementation sets the operator on the
  /// wrapped MFEM solver
  virtual void setOperator(mfem::OperatorHandle & op);

  /// Returns whether or not this solver (or its preconditioner) uses LOR
  bool isLOR() const { return _lor || (_preconditioner && _preconditioner->isLOR()); }

  /// For eigensolvers, this method calls the underlying Solve method
  virtual void solve() { mooseError("'solve' method not used in this solver type."); }

protected:
  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual void checkSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  /// Variable defining whether to use LOR solver
  bool _lor;

  /// Preconditioner to be used for the problem
  std::shared_ptr<LinearSolverBase> _preconditioner;

private:
  friend class ::MFEMProblemSolve;
};
} // namespace Moose::MFEM

#endif
