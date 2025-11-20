#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMFESpace.h"
#include "MFEMGeneralUserObject.h"

/**
 * Constructs and stores an mfem::ParComplexGridFunction object.
 */
class MFEMComplexVariable : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMComplexVariable(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed gridfunction.
  inline std::shared_ptr<mfem::ParComplexGridFunction> getComplexGridFunction() const
  {
    return _cmplx_gridfunction;
  }

  /// Returns a reference to the fespace used by the gridfunction.
  inline const MFEMFESpace & getFESpace() const { return _fespace; }

protected:
  const MFEMFESpace & _fespace;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParComplexGridFunction> buildComplexGridFunction();

  /// Stores the constructed gridfunction.
  const std::shared_ptr<mfem::ParComplexGridFunction> _cmplx_gridfunction{nullptr};
};

#endif
