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

#include "MFEMConstraint.h"

/**
 * Base class for applying essential volumetric constraints to the trial
 * ParGridFunction of an MFEM problem. Derived classes overwrite the values a
 * solution takes within the subdomain(s) given by 'block' and report the true
 * degrees of freedom that must be eliminated from the discrete system. This is
 * the volumetric analogue of MFEMEssentialBC.
 */
class MFEMEssentialConstraint : public MFEMConstraint
{
public:
  static InputParameters validParams();

  MFEMEssentialConstraint(const InputParameters & parameters);

  /// Apply the essential constraint, overwriting the values of gridfunc in the
  /// subdomain as desired and appending the constrained true dofs.
  virtual void ApplyConstraint(mfem::ParGridFunction & gridfunc,
                               mfem::Array<int> & ess_tdof_list) = 0;
};

#endif
