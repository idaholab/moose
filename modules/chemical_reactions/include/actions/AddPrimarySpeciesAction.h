//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDPRIMARYSPECIESACTION_H
#define ADDPRIMARYSPECIESACTION_H

#include "Action.h"

class AddPrimarySpeciesAction;

template <>
InputParameters validParams<AddPrimarySpeciesAction>();

class AddPrimarySpeciesAction : public Action
{
public:
  AddPrimarySpeciesAction(const InputParameters & params);

  virtual void act() override;

private:
  const std::vector<NonlinearVariableName> _vars;
};

#endif // ADDPRIMARYSPECIESACTION_H
