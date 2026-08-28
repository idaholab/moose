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

class MFEMEssentialConstraint : public MFEMObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMEssentialConstraint(const InputParameters & parameters);
  virtual ~MFEMEssentialConstraint() = default;

  /// Get name of the trial variable (gridfunction) the constraint acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _trial_var_name; }

  // Apply the essential constraint, overwriting the values of gridfunc in the subdomain as desired.
  virtual void ApplyConstraint(mfem::ParGridFunction & gridfunc,
                               mfem::Array<int> & ess_tdof_list) = 0;

protected:
  /// Name of the trial variable the constraint is applied to.
  const VariableName & _trial_var_name;
};

#endif
