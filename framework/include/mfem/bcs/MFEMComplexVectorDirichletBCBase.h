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

#include "MFEMComplexEssentialBC.h"

class MFEMComplexVectorDirichletBCBase : public MFEMComplexEssentialBC
{
public:
  static InputParameters validParams();

  ~MFEMComplexVectorDirichletBCBase() override = default;

protected:
  MFEMComplexVectorDirichletBCBase(const InputParameters & parameters);
  mfem::VectorCoefficient & _vec_coef_real;
  mfem::VectorCoefficient & _vec_coef_imag;
};

#endif
