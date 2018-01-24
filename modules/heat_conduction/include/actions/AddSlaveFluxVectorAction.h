//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDSLAVEFLUXVECTORACTION_H
#define ADDSLAVEFLUXVECTORACTION_H

#include "MooseObjectAction.h"

#include <string>

class AddSlaveFluxVectorAction : public Action
{
public:
  AddSlaveFluxVectorAction(const InputParameters & params);

  virtual void act();
};

template <>
InputParameters validParams<AddSlaveFluxVectorAction>();

#endif // ADDSLAVEFLUXVECTORACTION_H
