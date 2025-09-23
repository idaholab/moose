//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class AbaqusUELMesh;

class AddUELBCs : public Action
{
public:
  static InputParameters validParams();

  AddUELBCs(const InputParameters & params);

  virtual void act() override;

private:
  AbaqusUELMesh * _uel_mesh;

  std::shared_ptr<AbaqusUELStepUserObject> _step_uo;
};
