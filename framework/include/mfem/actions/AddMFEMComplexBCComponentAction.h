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

#include "MooseObjectAction.h"
#include "MFEMProblem.h"
/**
 * This class allows us to introduce as auxBCs the integrated BCs that will be
 * the real and imaginary components of MFEMComplexIntegratedBCs
 *
 */
class AddMFEMComplexBCComponentAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddMFEMComplexBCComponentAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
