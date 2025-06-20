//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMBoundaryRestrictable.h"
#include "MFEMContainers.h"
#include "Function.h"

class MFEMBoundaryCondition : public MFEMGeneralUserObject, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMBoundaryCondition(const InputParameters & parameters);
  virtual ~MFEMBoundaryCondition() = default;

  // Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
};

#endif
