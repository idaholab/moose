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

  /// Returns a reference to the fespace used by the gridfunction.
  inline const MFEMFESpace & getFESpace() const { return _fespace; }

protected:
  const MFEMFESpace & _fespace;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> buildGridFunction();

  /// Stores the constructed gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> _gridfunction{nullptr};
};

#endif
