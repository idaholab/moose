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
 * This class sets all EquationSystems in this problem to solve
 */
class SetMFEMEquationSystemsAction : public Action
{
public:
  static InputParameters validParams();

  SetMFEMEquationSystemsAction(const InputParameters & parameters);

  void act() override;
};

#endif
