//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarInitialConditionWarehouse.h"
#include "ScalarInitialCondition.h"

ScalarInitialConditionWarehouse::ScalarInitialConditionWarehouse()
  : MooseObjectWarehouseBase<ScalarInitialCondition>(false)
{
}

void
ScalarInitialConditionWarehouse::initialSetup()
{
  MooseObjectWarehouseBase<ScalarInitialCondition>::sort();
  for (const auto & ic : _active_objects[0])
    ic->initialSetup();
}
