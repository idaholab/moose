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

registerMooseObject("LevelSetApp", LevelSetMeshRefinementTransfer);

InputParameters
LevelSetMeshRefinementTransfer::validParams()
{
  InputParameters params = MultiAppCopyTransfer::validParams();
  params.addClassDescription("Transfers the mesh from the master application to the sub "
                             "application for the purposes of level set reinitialization problems "
                             "with mesh adaptivity.");
  params.suppressParameter<MultiMooseEnum>("direction");

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
  if (hasFromMultiApp())
    paramError("from_multi_app", "from_multiapp or between_multiapp transfers are not supported");
}

void
LevelSetMeshRefinementTransfer::initialSetup()
{
  FEProblemBase & from_problem = getToMultiApp()->problemBase();
  for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    if (getToMultiApp()->hasLocalApp(i))
    {
      FEProblemBase & to_problem = getToMultiApp()->appProblemBase(i);
      MooseVariable & to_var = to_problem.getStandardVariable(0, _to_var_name);
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
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      if (getToMultiApp()->hasLocalApp(i))
      {
        FEProblemBase & to_problem = getToMultiApp()->appProblemBase(i);
        Adaptivity & adapt = to_problem.adaptivity();
        adapt.setAdaptivityOn(true);
        to_problem.adaptMesh();
        adapt.setAdaptivityOn(false);
      }
  }
}
