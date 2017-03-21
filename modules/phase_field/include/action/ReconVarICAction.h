/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECONVARICACTION_H
#define RECONVARICACTION_H

#include "InputParameters.h"
#include "Action.h"

// Forward Declarations
class ReconVarICAction;

template <>
InputParameters validParams<ReconVarICAction>();

/**
 * Action to set up initial conditions for a set of order parameters using EBSDReader data
 */
class ReconVarICAction : public Action
{
public:
  ReconVarICAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _op_num;
  const std::string _var_name_base;
};

#endif // RECONVARICACTION_H
