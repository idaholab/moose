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

/// Real-valued finite element eigenproblem.
class MFEMEigenproblem : public MFEMEigenproblemBase
{
public:
  static InputParameters validParams();

  MFEMEigenproblem(const InputParameters & params);
  virtual ~MFEMEigenproblem() {}
};

#endif
