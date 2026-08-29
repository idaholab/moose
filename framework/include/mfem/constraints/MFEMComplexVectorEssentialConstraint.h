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
 * Complex (time-harmonic) counterpart of MFEMVectorEssentialConstraint. Strongly
 * constrains the real and imaginary parts of a complex vector variable in the
 * specified subdomain(s) to separate vector coefficients.
 */
class MFEMComplexVectorEssentialConstraint : public MFEMComplexEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMComplexVectorEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParComplexGridFunction & gridfunc,
                       mfem::Array<int> & ess_tdof_list) override;

protected:
  mfem::VectorCoefficient & _vec_coef_real;
  mfem::VectorCoefficient & _vec_coef_imag;
};

#endif
