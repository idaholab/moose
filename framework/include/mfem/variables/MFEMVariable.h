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

namespace Moose::MFEM
{
/**
 * Constructs and stores an mfem::ParGridFunction object.
 */
class Variable : public Object
{
public:
  static InputParameters validParams();

  Variable(const InputParameters & parameters);

  /// Returns a shared pointer to the constructed gridfunction.
  inline std::shared_ptr<mfem::ParGridFunction> getGridFunction() const { return _gridfunction; }

  /// Returns a reference to the MOOSE FESpace. Only valid when constructed via `fespace`;
  /// throws if the variable was constructed via `fespace_hierarchy`.
  inline const FESpace & getFESpace() const
  {
    mooseAssert(_fespace_ptr, "getFESpace() called on a hierarchy-backed variable");
    return *_fespace_ptr;
  }

  /// Returns true if the variable lives on a scalar (vdim == 1) finite element space.
  bool isScalar() const;

  /// Returns the variable name corresponding to the time derivative of the Moose::MFEM::Variable.
  inline const VariableName & getTimeDerivativeName() const { return _time_derivative_name; }

protected:
  /// Non-owning pointer to the MOOSE FESpace; null when using fespace_hierarchy.
  const FESpace * _fespace_ptr{nullptr};
  /// The underlying MFEM FESpace — always populated regardless of which parameter was used.
  std::shared_ptr<mfem::ParFiniteElementSpace> _par_fespace;

private:
  /// Constructs the gridfunction.
  const std::shared_ptr<mfem::ParGridFunction> buildGridFunction();

  /// Stores the constructed gridfunction.
  std::shared_ptr<mfem::ParGridFunction> _gridfunction{nullptr};

  /// Optional name of the time derivative to associate with this variable in transient problems.
  const VariableName _time_derivative_name;
};

} // namespace Moose::MFEM
#endif
