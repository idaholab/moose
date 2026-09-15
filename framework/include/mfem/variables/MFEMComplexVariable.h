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
#include "MFEMObject.h"

/**
 * Constructs and stores an mfem::ParComplexGridFunction object.
 */
class MFEMComplexVariable : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMComplexVariable(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed gridfunction.
  std::shared_ptr<mfem::ParComplexGridFunction> getComplexGridFunction() const
  {
    return _cmplx_gridfunction;
  }

  /// Returns a reference to the fespace used by the gridfunction.
  const MFEMFESpace & getFESpace() const { return _fespace; }

  // Declare default coefficients associated with this complex gridfunction.
  void declareCoefficients();

protected:
  const MFEMFESpace & _fespace;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParComplexGridFunction> buildComplexGridFunction();

  /// Stores the constructed gridfunction.
  const std::shared_ptr<mfem::ParComplexGridFunction> _cmplx_gridfunction{nullptr};
};

#endif
