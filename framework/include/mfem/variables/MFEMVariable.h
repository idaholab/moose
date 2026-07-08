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
 * Constructs and stores an mfem::ParGridFunction object.
 */
class MFEMVariable : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMVariable(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed gridfunction.
  std::shared_ptr<mfem::ParGridFunction> getGridFunction() const { return _gridfunction; }

  /// Returns a reference to the fespace used by the gridfunction. Only valid when constructed via
  /// `fespace`; asserts if the variable was constructed via `fespace_hierarchy`.
  const MFEMFESpace & getFESpace() const
  {
    mooseAssert(_fespace_ptr, "getFESpace() called on a hierarchy-backed variable");
    return *_fespace_ptr;
  }

  /// Returns the variable name corresponding to the time derivative of the MFEMVariable.
  const VariableName & getTimeDerivativeName() const { return _time_derivative_name; }

  /// Declare default coefficients associated with this gridfunction
  void declareCoefficients();

protected:
  /// Non-owning pointer to the MOOSE FESpace; null when using fespace_hierarchy.
  const MFEMFESpace * _fespace_ptr = nullptr;
  /// The underlying MFEM FESpace - always populated regardless of which parameter was used.
  std::shared_ptr<mfem::ParFiniteElementSpace> _par_fespace;
  bool _is_scalar = false;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> buildGridFunction();

  /// Stores the constructed gridfunction.
  std::shared_ptr<mfem::ParGridFunction> _gridfunction = nullptr;

  /// Optional name of the time derivative to associate with this variable in transient problems.
  const VariableName _time_derivative_name;
};

#endif
