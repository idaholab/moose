//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AppendMeshGeneratorAction.h"
#include "MooseMesh.h"
#include "MeshGenerator.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseAction("MooseTestApp", AppendMeshGeneratorAction, "append_mesh_generator");
registerMooseAction("MooseTestApp", AppendMeshGeneratorAction, "add_mesh_generator");

InputParameters
AppendMeshGeneratorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  return params;
}

AppendMeshGeneratorAction::AppendMeshGeneratorAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AppendMeshGeneratorAction::act()
{
  if (!_mesh)
    mooseError("No mesh file was supplied and no generation block was provided");

  // This if statement lets us test adding at the wrong time if it's a TestMeshGenerator
  if (_type == "TestMeshGenerator" || _current_task == "append_mesh_generator")
    _app.appendMeshGenerator(_type, _name, _moose_object_pars);
}
