//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELStepUserObject.h"
#include "AddUELBCs.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AbaqusUELMesh.h"
#include "MooseTypes.h"
#include <dirent.h>
#include <memory>

registerMooseAction("SolidMechanicsApp", AddUELBCs, "add_user_object");
registerMooseAction("SolidMechanicsApp", AddUELBCs, "add_bc");
registerMooseAction("SolidMechanicsApp", AddUELBCs, "add_nodal_kernel");

InputParameters
AddUELBCs::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add boundary conditions from an Abaqus input");
  return params;
}

AddUELBCs::AddUELBCs(const InputParameters & params) : Action(params), _uel_mesh(nullptr) {}

void
AddUELBCs::act()
{
  const std::string step_uo_name = "abaqus_step_uo";

  // set up step user object
  if (_current_task == "add_user_object")
  {
    std::string uo_type = "AbaqusUELStepUserObject";
    InputParameters params = _factory.getValidParams(uo_type);
    const auto uos = _problem->addUserObject(uo_type, step_uo_name, params);
    // we remember the thread 0 copy
    mooseAssert(uos.size() > 0, "Error constructing the step user object");
    _step_uo = std::dynamic_pointer_cast<AbaqusUELStepUserObject>(uos[0]);
    return;
  }

  _uel_mesh = dynamic_cast<AbaqusUELMesh *>(_mesh.get());
  mooseAssert(_step_uo, "Step user object should have been constructed, but it wasn't.");
  const auto var_list = _step_uo->getVariables();

  // create boundary conditions
  if (_current_task == "add_bc")
  {
    for (const auto & [var_id, var_name] : var_list)
    {
      std::string bc_type = "AbaqusEssentialBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<Abaqus::AbaqusID>("abaqus_var_id") = var_id;
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<UserObjectName>("step_user_object") = step_uo_name;
      _problem->addBoundaryCondition(bc_type, "abaqus_essential_bc_" + var_name, params);
    }
  }

  // create nodal kernels for deactivated BCs
  if (_current_task == "add_nodal_kernel")
  {
    for (const auto & [var_id, var_name] : var_list)
    {
      std::string nk_type = "AbaqusForceBC";
      InputParameters params = _factory.getValidParams(nk_type);
      params.set<Abaqus::AbaqusID>("abaqus_var_id") = var_id;
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<UserObjectName>("step_user_object") = step_uo_name;
      _problem->addNodalKernel(nk_type, "abaqus_force_bc_" + var_name, params);
    }
  }
}
