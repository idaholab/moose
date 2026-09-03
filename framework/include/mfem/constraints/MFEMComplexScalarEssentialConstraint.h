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

#include "MFEMComplexEssentialConstraint.h"

/**
 * Complex (time-harmonic) counterpart of MFEMScalarEssentialConstraint. Strongly
 * constrains the real and imaginary parts of a complex scalar variable in the
 * specified subdomain(s) to separate coefficients.
 */
class MFEMComplexScalarEssentialConstraint : public MFEMComplexEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMComplexScalarEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                       mfem::Array<int> & ess_tdof_list) override;

protected:
  mfem::Coefficient & _coef_real;
  mfem::Coefficient & _coef_imag;
};

#endif
