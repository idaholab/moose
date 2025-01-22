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

class InitialConditionBase;

/**
 * Warehouse for storing initial conditions
 */
class InitialConditionWarehouse : public MooseObjectWarehouseBase<InitialConditionBase>
{
public:
  InitialConditionWarehouse();

  /**
   * Initial setup
   */
  void initialSetup(THREAD_ID tid);

  /**
   * Add object to the warehouse.
   */
  void addObject(std::shared_ptr<InitialConditionBase> object, THREAD_ID tid, bool recurse = true);

  /**
   * Get a list of dependent UserObjects for this exec type
   * @return a set of dependent user objects
   */
  std::set<std::string> getDependObjects() const;

protected:
  ///@{
  /// Maps used to check if multiple ICs define the same variable on the same block/boundary for the same state (e.g CURRENT, OLD, OLDER).
  /// They are vectors of maps because each map is repeated for each thread.
  /// Each map relates string-int tuples to a set of block/boundary IDs.
  /// The string-int tuple is a unique identifier for a specific variable and state. The string-int tuple is renamed to ic_key_type for clarity.
  /// The algorithm then makes sure that a new IC object does not overlap with a previous IC object (i.e. same block/boundary).
  using ic_key_type = std::tuple<VariableName, unsigned short>;
  std::vector<std::map<ic_key_type, std::set<BoundaryID>>> _boundary_ics;
  std::vector<std::map<ic_key_type, std::set<SubdomainID>>> _block_ics;
  ///@}
};
