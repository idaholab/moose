//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDLOTSOFDIFFUSION_H
#define ADDLOTSOFDIFFUSION_H

#include "AddVariableAction.h"

class AddLotsOfDiffusion;

template <>
InputParameters validParams<AddLotsOfDiffusion>();

class AddLotsOfDiffusion : public AddVariableAction
{
public:
  AddLotsOfDiffusion(const InputParameters & params);

  virtual void act();
};

#endif // ADDLOTSOFDIFFUSION_H
