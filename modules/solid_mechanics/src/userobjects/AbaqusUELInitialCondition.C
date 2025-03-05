//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELInitialCondition.h"
#include "AbaqusUELMesh.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("SolidMechanicsApp", AbaqusUELInitialCondition);

InputParameters
AbaqusUELInitialCondition::validParams()
{
  auto params = NodalUserObject::validParams();
  params.addClassDescription("Add initial conditions from an Abaqus input");
  return params;
}

AbaqusUELInitialCondition::AbaqusUELInitialCondition(const InputParameters & params)
  : NodalUserObject(params), _uel_mesh(dynamic_cast<AbaqusUELMesh *>(&_mesh))
{
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");
}

void
AbaqusUELInitialCondition::execute()
{
  std::cout << "on node " << _current_node->id() << '\n';
}
