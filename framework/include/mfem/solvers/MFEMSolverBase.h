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
#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include <memory>

/**
 * Base class for wrapping mfem::Solver-derived classes.
 */
class MFEMSolverBase : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMSolverBase(const InputParameters & parameters);

  /// Retrieves the preconditioner userobject if present, sets the member pointer to
  /// said object if still unset, and sets the solver to use this preconditioner.
  template <typename T>
  void setPreconditioner(T & solver);

  /// Returns the wrapped MFEM solver
  mfem::Solver & getSolver();

  /// Updates the solver with the given bilinear form and essential dof list, in case an LOR or algebraic solver is needed.
  virtual void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) = 0;

  /// Returns whether or not this solver (or its preconditioner) uses LOR
  bool isLOR() const { return _lor || (_preconditioner && _preconditioner->isLOR()); }

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters & parameters) = 0;

  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual bool checkSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  // Variable defining whether to use LOR solver
  bool _lor;

  // Solver and preconditioner to be used for the problem
  std::unique_ptr<mfem::Solver> _solver;
  MFEMSolverBase * _preconditioner;
};

inline mfem::Solver &
MFEMSolverBase::getSolver()
{
  mooseAssert(_solver, "Attempting to retrieve solver before it's been constructed");
  return *_solver;
}

#endif
