#ifdef MFEM_ENABLED

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

  /// Returns a shared pointer to the instance of the Solver derived-class.
  virtual std::shared_ptr<mfem::Solver> getSolver() { return _solver; }

  /// Updates the solver with the given bilinear form and essential dof list, in case an LOR or algebraic solver is needed.
  virtual void updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs) = 0;

  bool isLOR() const { return _lor; }

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters & parameters) = 0;

  // Variable defining whether to use LOR solver
  bool _lor;

  // Solver to be used for the problem
  std::shared_ptr<mfem::Solver> _solver{nullptr};
};

#endif
