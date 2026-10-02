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
 * Strongly constrains a vector variable in the specified subdomain(s) to a
 * vector coefficient. Works for vector H1 as well as H(curl) and H(div) spaces.
 */
class MFEMVectorEssentialConstraint : public MFEMEssentialConstraint
{
public:
  static InputParameters validParams();

  MFEMVectorEssentialConstraint(const InputParameters & parameters);

  void ApplyConstraint(mfem::ParGridFunction & gridfunc, mfem::Array<int> & ess_tdof_list) override;

protected:
  mfem::VectorCoefficient & _vec_coef;
};

#endif
