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

#include "AddVariableAction.h"

class AddAuxVariableAction;

template<>
InputParameters validParams<AddAuxVariableAction>();

class AddAuxVariableAction : public AddVariableAction
{
public:
  AddAuxVariableAction(const std::string & name, InputParameters params);

  static MooseEnum getAuxVariableFamilies();
  static MooseEnum getAuxVariableOrders();
};

#endif // ADDAUXVARIABLEACTION_H
