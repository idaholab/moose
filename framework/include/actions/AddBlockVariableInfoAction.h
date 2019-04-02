//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDBLOCKVARIABLEINFOACTION_H
#define ADDBLOCKVARIABLEINFOACTION_H

// MOOSE includes
#include "Action.h"

#include "libmesh/fe_type.h"

// Forward declerations
class AddBlockVariableInfoAction;

template <>
InputParameters validParams<AddBlockVariableInfoAction>();

/**
 * Adds nonlinear variable
 */
class AddBlockVariableInfoAction : public Action
{
public:
  /**
   * Class constructor
   */
  AddBlockVariableInfoAction(InputParameters params);

  virtual void act() override;

protected:
  /**
   * Get the block ids from the input parameters
   * @return A set of block ids defined in the input
   */
  std::set<SubdomainID> getSubdomainIDs();

  /// FEType for the variable being created
  FEType _fe_type;

  /// True if the variable being created is a scalar
  bool _scalar_var;

  /// All block ids
  std::set<SubdomainID> _all_block_ids;
};

#endif // ADDBLOCKVARIABLEINFOACTION_H
