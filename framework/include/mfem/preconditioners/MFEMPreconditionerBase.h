#pragma once
#include "MFEMGeneralUserObject.h"
#include "mfem.hpp"
#include <memory>

/**
 * Base class for wrapping mfem::Solver-derived classes.
 */
class MFEMPreconditionerBase : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMPreconditionerBase(const InputParameters & parameters);

  /// Returns a shared pointer to the instance of the Solver derived-class.
  virtual std::shared_ptr<mfem::Solver> getPreconditioner() const = 0;

protected:
  /// Override in derived classes to construct and set the preconditioner options.
  virtual void constructPreconditioner(const InputParameters & parameters) = 0;
};
