//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
