//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AddVariableAction.h"

/**
 * Action for creating Auxiliary variables
 */
class AddAuxVariableAction : public AddVariableAction
{
public:
  /**
   * Class constructor
   */
  static InputParameters validParams();

  AddAuxVariableAction(const InputParameters & params);

  /**
   * Returns available families for AuxVariables
   * @return MooseEnum with the valid familes for AuxVariables
   */
  static MooseEnum getAuxVariableFamilies();

  /**
   * Returns available orders for AuxVariables
   * @return MooseEnum with valid orders
   */
  static MooseEnum getAuxVariableOrders();

protected:
  void init() override;
};
