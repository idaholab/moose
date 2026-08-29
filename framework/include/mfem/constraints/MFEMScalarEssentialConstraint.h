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
 * Strongly constrains a scalar variable in the specified subdomain(s) to a scalar
 * coefficient.
 */
class MFEMScalarEssentialConstraint : public MFEMEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMScalarEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;

protected:
  mfem::Coefficient & _coef;
};

#endif
