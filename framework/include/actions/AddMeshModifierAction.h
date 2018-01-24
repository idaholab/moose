//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDMESHMODIFIERACTION_H
#define ADDMESHMODIFIERACTION_H

#include "MooseObjectAction.h"

// Forward Declaration
class AddMeshModifierAction;

template <>
InputParameters validParams<AddMeshModifierAction>();

class AddMeshModifierAction : public MooseObjectAction
{
public:
  AddMeshModifierAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDMESHMODIFIERACTION_H
