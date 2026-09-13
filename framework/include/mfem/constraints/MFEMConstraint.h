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

#include "MFEMObject.h"
#include "MFEMBlockRestrictable.h"

/**
 * Base class for objects constraining the value an MFEM variable may take within
 * one or more mesh subdomains, given by 'block'. It holds everything that does not
 * depend on whether the variable is real or complex; the two families of
 * constraint derive from it separately and each declares its own ApplyConstraint
 * taking the grid function type it acts on:
 *
 * - MFEMEssentialConstraint        (mfem::ParGridFunction)
 * - MFEMComplexEssentialConstraint (mfem::ParComplexGridFunction)
 *
 * This mirrors how MFEMBoundaryCondition relates to MFEMEssentialBC and
 * MFEMComplexEssentialBC.
 */
class MFEMConstraint : public MFEMObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMConstraint(const InputParameters & parameters);
  virtual ~MFEMConstraint() = default;

  /// Name of the trial variable (gridfunction) the constraint acts on.
  virtual const std::string & getTrialVariableName() const { return _trial_var_name; }

protected:
  /// Name of the trial variable the constraint is applied to.
  const VariableName & _trial_var_name;
};

#endif
