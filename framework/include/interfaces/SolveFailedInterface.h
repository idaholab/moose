//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

/**
 * Interface for notifying objects that the solve has failed
 */
class SolveFailedInterface
{
public:
  SolveFailedInterface(const MooseObject * moose_object);
  virtual ~SolveFailedInterface() = default;

  /**
   * Called on this object immediately after the solve failed
   */
  virtual void onSolveFailed() {}
};
