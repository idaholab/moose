/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MORTARPERIODICACTION_H
#define MORTARPERIODICACTION_H

#include "Action.h"

class MortarPeriodicAction;

template <>
InputParameters validParams<MortarPeriodicAction>();

/**
 * Set up Mortar based periodicity in an input file with a MortarPeriodicMesh
 */
class MortarPeriodicAction : public Action
{
public:
  MortarPeriodicAction(const InputParameters & parameters);

  virtual void act();

protected:
  // all variables this action operates on
  std::vector<VariableName> _variables;

  // type of the periodic constraint to apply (value, gradient)
  const unsigned int _periodicity;
};

#endif // MORTARPERIODICACTION_H
