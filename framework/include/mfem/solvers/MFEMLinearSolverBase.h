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

  /// Updates the solver with the given bilinear form and essential dof list, in case an LOR or
  /// algebraic solver is needed.
  virtual void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) = 0;

  /// Returns whether or not this solver (or its preconditioner) uses LOR
  bool isLOR() const { return _lor || (_preconditioner && _preconditioner->isLOR()); }

protected:
  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual void checkSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  /// Variable defining whether to use LOR solver
  bool _lor;

  /// Preconditioner to be used for the problem
  LinearSolverBase * _preconditioner;
};
} // namespace Moose::MFEM

#endif
