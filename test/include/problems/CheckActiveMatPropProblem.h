//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

class AuxiliarySystem;

class CheckActiveMatPropProblem : public FEProblem
{
public:
  static InputParameters validParams();

  CheckActiveMatPropProblem(const InputParameters & params);

  /**
   * Get the material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  std::unordered_set<unsigned int> getActiveMaterialProperties(const THREAD_ID tid) const;

private:
  std::shared_ptr<AuxiliarySystem> _test_aux;
};
