//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectWarehouseBase.h"
#include "MooseTypes.h"

class FVInitialConditionBase;

/**
 * Warehouse for storing finite volume initial conditions
 */
class FVInitialConditionWarehouse : public MooseObjectWarehouseBase<FVInitialConditionBase>
{
public:
  FVInitialConditionWarehouse();

  /**
   * Initial setup
   */
  void initialSetup(THREAD_ID tid);

  /**
   * Add object to the warehouse.
   */
  void
  addObject(std::shared_ptr<FVInitialConditionBase> object, THREAD_ID tid, bool recurse = true);

protected:
  ///@{
  /// Variable name to block IDs for error checking
  std::vector<std::map<std::string, std::set<SubdomainID>>> _block_ics;
  ///@}
};
