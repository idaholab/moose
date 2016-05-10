/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIAUXVARIABLESACTION_H
#define MULTIAUXVARIABLESACTION_H

#include "AddAuxVariableAction.h"

/**
 * Automatically generates all auxvariables given vectors telling it the names
 * and how many to create
 */
class MultiAuxVariablesAction: public AddAuxVariableAction
{
public:
  MultiAuxVariablesAction(InputParameters params);

  virtual void act();

  std::vector<unsigned int> _op_num;
  std::vector<std::string> _var_name_base;
};

template<>
InputParameters validParams<MultiAuxVariablesAction>();

#endif //MULTIAUXVARIABLESACTION_H
