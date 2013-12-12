/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ADDAUXVARIABLEACTION_H
#define ADDAUXVARIABLEACTION_H

// MOOSE includes
#include "AddVariableAction.h"

//Forward declerations
class AddAuxVariableAction;

template<>
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
  AddAuxVariableAction(const std::string & name, InputParameters params);

  /**
   * Creates the AuxVariable
   */
  virtual void act();

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
