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
class MultiAuxVariablesAction : public AddAuxVariableAction
{
public:
  MultiAuxVariablesAction(InputParameters params);

  virtual void act();

protected:
  /// number of grains to create
  const unsigned int _grain_num;

  /// base name for the auxvariables
  const std::vector<std::string> & _var_name_base;

  /// number of auxvariables
  const unsigned int _num_var;

  /// list of material properties to be used
  const MultiMooseEnum & _data_type;

  /// number of properties
  const unsigned int _data_size;
};

template <>
InputParameters validParams<MultiAuxVariablesAction>();

#endif // MULTIAUXVARIABLESACTION_H
