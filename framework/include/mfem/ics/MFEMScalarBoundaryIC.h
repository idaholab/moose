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
#include "MFEMInitialCondition.h"

/**
 * Class used to set the initial value(s) on a scalar valued MFEMVariable.
 */
class MFEMScalarBoundaryIC : public MFEMInitialCondition, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();
  MFEMScalarBoundaryIC(const InputParameters & params);
  virtual void execute() override;
};

#endif
