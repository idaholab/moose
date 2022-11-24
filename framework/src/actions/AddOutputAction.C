//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddOutputAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "OutputWarehouse.h"
#include "Output.h"
#include "MooseApp.h"
#include "FileMesh.h"
#include "MooseApp.h"

#include "libmesh/mesh_function.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/explicit_system.h"

registerMooseAction("MooseApp", AddOutputAction, "add_output");

InputParameters
AddOutputAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Action responsible for creating Output objects.");
  // This is to help with input file validation.  FEProblem adds file_base to
  // all objects that don't have it set from the input file.
  params.set<std::vector<std::string>>("_object_params_set_by_action") = {"file_base"};
  return params;
}

AddOutputAction::AddOutputAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddOutputAction::act()
{
  if (_current_task == "add_output")
    _problem->addOutput(_type, _name, _moose_object_pars);
}
