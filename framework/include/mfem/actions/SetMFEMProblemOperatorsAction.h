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

/**
 * This class sets all ProblemOperators in this problem to solve
 */
class SetMFEMProblemOperatorsAction : public Action
{
public:
  static InputParameters validParams();

  SetMFEMProblemOperatorsAction(const InputParameters & parameters);

  void act() override;
};

#endif
