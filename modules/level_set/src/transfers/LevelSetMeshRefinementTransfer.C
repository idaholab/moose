/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LevelSetMeshRefinementTransfer.h"

// MOOSE includes
#include "Adaptivity.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "MultiApp.h"

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

  MooseUtils::setExecuteOnFlags(params, 1, EXEC_CUSTOM);
  params.set<bool>("check_multiapp_execute_on") = false;
  params.suppressParameter<MultiMooseEnum>("execute_on");

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
      adapt.setCyclesPerStep(1);
      adapt.init(0, 0);
      adapt.setUseNewSystem();
      adapt.setMaxHLevel(from_problem.adaptivity().getMaxHLevel());
      adapt.setAdpaptivityOn(false);
    }
}

void
LevelSetMeshRefinementTransfer::execute()
{
  MultiAppCopyTransfer::execute();

  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i))
    {
      FEProblemBase & to_problem = _multi_app->appProblemBase(i);
      Adaptivity & adapt = to_problem.adaptivity();
      adapt.setAdpaptivityOn(true);
      to_problem.adaptMesh();
      adapt.setAdpaptivityOn(false);
    }
}
