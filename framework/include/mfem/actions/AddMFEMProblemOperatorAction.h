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

#include "Action.h"
#include "MFEMProblem.h"
/**
 * This class implements the action controlling the construction of the
 * required MFEM ProblemOperator.
 */
class AddMFEMProblemOperatorAction : public Action
{
public:
  static InputParameters validParams();

  AddMFEMProblemOperatorAction(const InputParameters & parameters);

  virtual void act() override;
};

#endif
