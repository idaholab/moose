//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * This Action retrieves all of the Actions from the MooseAction Warehouse and triggers the
 * addRelationshipManagers() call on each of them. Additionally, it is responsible for triggering
 * the attachment of those relationship managers to the relevant libMesh objects.
 */
class AddRelationshipManager : public Action
{
public:
  static InputParameters validParams();

  AddRelationshipManager(const InputParameters & params);

  virtual void act() override;
};
