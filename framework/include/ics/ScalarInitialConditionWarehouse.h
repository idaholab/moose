//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectWarehouseBase.h"
#include "MooseTypes.h"

class ScalarInitialCondition;

/**
 * Warehouse for storing scalar initial conditions
 */
class ScalarInitialConditionWarehouse : public MooseObjectWarehouseBase<ScalarInitialCondition>
{
public:
  ScalarInitialConditionWarehouse();

  /**
   * Initial setup
   */
  void initialSetup();
};
