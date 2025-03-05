#pragma once

#include "MFEMFESpace.h"
#include "MFEMGeneralUserObject.h"

/**
 * Constructs and stores an mfem::ParGridFunction object.
 */
class MFEMVariable : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMVariable(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed gridfunction.
  inline std::shared_ptr<mfem::ParGridFunction> getGridFunction() const { return _gridfunction; }

protected:
  const MFEMFESpace & _fespace;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> buildGridFunction();

  /// Stores the constructed gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> _gridfunction{nullptr};
};
