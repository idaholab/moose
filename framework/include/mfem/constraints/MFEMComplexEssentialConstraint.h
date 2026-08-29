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

#include "MFEMEssentialConstraint.h"

/**
 * Base class for applying essential volumetric constraints to the trial
 * ParComplexGridFunction of a complex (time-harmonic) MFEM problem. The
 * constraint acts on the real and imaginary components together.
 */
class MFEMComplexEssentialConstraint : public MFEMEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMComplexEssentialConstraint(const InputParameters & parameters);

  /// Real-valued entry point is unused for complex problems.
  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;

  /// Apply the essential constraint, overwriting values of gridfunc as desired.
  virtual void ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                               mfem::Array<int> & ess_tdof_list) = 0;
};

#endif
