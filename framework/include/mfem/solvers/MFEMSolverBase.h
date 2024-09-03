#pragma once
#include "MFEMGeneralUserObject.h"
#include "mfem.hpp"
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
  virtual std::shared_ptr<mfem::Solver> getSolver() const = 0;

protected:
  /// Override in derived classes to construct and set the solver options.
  virtual void constructSolver(const InputParameters & parameters) = 0;
};
