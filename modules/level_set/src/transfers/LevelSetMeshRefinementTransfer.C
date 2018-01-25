//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetMeshRefinementTransfer.h"

// MOOSE includes
#include "Adaptivity.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "MultiApp.h"
#include "LevelSetTypes.h"

template <>
InputParameters
validParams<LevelSetMeshRefinementTransfer>()
{
  InputParameters params = validParams<MultiAppCopyTransfer>();
  params.addClassDescription("Transfers the mesh from the master application to the sub "
                             "application for the purposes of level set reinitialization problems "
                             "with mesh adaptivity.");
  params.set<MooseEnum>("direction") = "TO_MULTIAPP";
  params.suppressParameter<MooseEnum>("direction");

  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(LevelSet::EXEC_ADAPT_MESH, LevelSet::EXEC_COMPUTE_MARKERS);
  exec = {LevelSet::EXEC_COMPUTE_MARKERS, LevelSet::EXEC_ADAPT_MESH};
  params.set<bool>("check_multiapp_execute_on") = false;
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

LevelSetMeshRefinementTransfer::LevelSetMeshRefinementTransfer(const InputParameters & parameters)
  : MultiAppCopyTransfer(parameters)
{
}

void
LevelSetMeshRefinementTransfer::initialSetup()
{
  FEProblemBase & from_problem = _multi_app->problemBase();
  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & to_problem = _multi_app->appProblemBase(i);
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      Adaptivity & adapt = to_problem.adaptivity();
      adapt.setMarkerVariableName(to_var.name());
      adapt.setCyclesPerStep(from_problem.adaptivity().getCyclesPerStep());
      adapt.init(1, 0);
      adapt.setUseNewSystem();
      adapt.setMaxHLevel(from_problem.adaptivity().getMaxHLevel());
      adapt.setAdaptivityOn(false);
    }
}

void
LevelSetMeshRefinementTransfer::execute()
{
  if (_current_execute_flag == LevelSet::EXEC_COMPUTE_MARKERS)
    MultiAppCopyTransfer::execute();

  else if (_current_execute_flag == LevelSet::EXEC_ADAPT_MESH)
  {
    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      if (_multi_app->hasLocalApp(i))
      {
        FEProblemBase & to_problem = _multi_app->appProblemBase(i);
        Adaptivity & adapt = to_problem.adaptivity();
        adapt.setAdaptivityOn(true);
        to_problem.adaptMesh();
        adapt.setAdaptivityOn(false);
      }
  }
}
