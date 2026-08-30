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
 * ParComplexGridFunction of a complex (time-harmonic) MFEM problem. The
 * constraint acts on the real and imaginary components together.
 *
 * A sibling of MFEMEssentialConstraint rather than a derived class: the two act
 * on different grid function types, so neither can implement the other's
 * interface.
 */
class MFEMComplexEssentialConstraint : public MFEMConstraint
{
public:
  static InputParameters validParams();

  MFEMComplexEssentialConstraint(const InputParameters & parameters);

  /// Apply the essential constraint, overwriting the values of gridfunc in the
  /// subdomain as desired and appending the constrained true dofs.
  virtual void ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                               mfem::Array<int> & ess_tdof_list) = 0;
};

#endif
