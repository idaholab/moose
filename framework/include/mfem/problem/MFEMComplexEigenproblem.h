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

#include "MFEMEigenproblemBase.h"

/// Complex-valued finite element eigenproblem. The complex (Hermitian) system is assembled in its
/// monolithic real form and solved with the real eigensolvers; eigenmodes are stored as complex
/// grid functions.
class MFEMComplexEigenproblem : public MFEMEigenproblemBase
{
public:
  static InputParameters validParams();

  MFEMComplexEigenproblem(const InputParameters & params);
  virtual ~MFEMComplexEigenproblem() {}
};

#endif
