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
  virtual std::shared_ptr<mfem::Solver> getSolver() = 0;

  /// Updates the solver with the given bilinear form and essential dof list, in case an LOR or algebraic solver is needed.
  /// The solver can optionally be updated with a preconditioner. If a preconditioner is not needed, pass a nullptr.
  virtual void updateSolver(mfem::ParBilinearForm &a, mfem::Array<int> &tdofs,
    std::shared_ptr<mfem::Solver> &solver, std::shared_ptr<mfem::Solver> preconditioner = nullptr) const{};


protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters & parameters) = 0;
};

#endif
