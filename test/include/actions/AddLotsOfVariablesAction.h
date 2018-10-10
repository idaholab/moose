// ADDLOTSOFSCALARVARIABLESACTION_H
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDLOTSOFVARIABLESACTION_H
#define ADDLOTSOFVARIABLESACTION_H

#include "Action.h"

class AddLotsOfVariablesAction;

template <>
InputParameters validParams<AddLotsOfVariablesAction>();

class AddLotsOfVariablesAction : public Action
{
public:
  AddLotsOfVariablesAction(const InputParameters & parameters);

  virtual void act();

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
};

#endif // ADDLOTSOFVARIABLESACTION_H
