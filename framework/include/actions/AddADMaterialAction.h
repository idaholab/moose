//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDADMATERIALACTION_H
#define ADDADMATERIALACTION_H

#include "MooseADObjectAction.h"

class AddADMaterialAction;

template <>
InputParameters validParams<AddADMaterialAction>();

class AddADMaterialAction : public MooseADObjectAction
{
public:
  AddADMaterialAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDADMATERIALACTION_H
