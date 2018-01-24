//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SETUPMESHACTION_H
#define SETUPMESHACTION_H

#include "MooseObjectAction.h"

// Forward Declaration
class SetupMeshAction;
class MooseMesh;

template <>
InputParameters validParams<SetupMeshAction>();

class SetupMeshAction : public MooseObjectAction
{
public:
  SetupMeshAction(InputParameters params);

  virtual void act() override;

private:
  void setupMesh(MooseMesh * mesh);
};

#endif // SETUPMESHACTION_H
