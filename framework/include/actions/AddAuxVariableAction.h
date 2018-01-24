//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDAUXVARIABLEACTION_H
#define ADDAUXVARIABLEACTION_H

// MOOSE includes
#include "AddVariableAction.h"

// Forward declarations
class AddAuxVariableAction;

template <>
InputParameters validParams<AddAuxVariableAction>();

/**
 * Action for creating Auxiliary variables
 */
class AddAuxVariableAction : public AddVariableAction
{
public:
  /**
   * Class constructor
   */
  AddAuxVariableAction(InputParameters params);

  virtual void act() override;

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
};

#endif // ADDAUXVARIABLEACTION_H
