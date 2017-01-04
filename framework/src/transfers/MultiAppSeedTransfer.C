/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MultiAppSeedTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"

template <>
InputParameters
validParams<MultiAppSeedTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.set<MultiMooseEnum>("execute_on") = "timestep_begin nonlinear timestep_end timestep_begin custom";
  return params;
}

MultiAppSeedTransfer::MultiAppSeedTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters)
{
}

void
MultiAppSeedTransfer::transfer(FEProblemBase & to_problem, FEProblemBase & from_problem)
{
  to_problem.seedMasterRand(from_problem.rand());
}

void
MultiAppSeedTransfer::execute()
{
  _console << "Beginning MultiAppSeedTransfer " << name() << std::endl;
  FEProblemBase & from_problem = _multi_app->problemBase();
  if (_direction != TO_MULTIAPP)
    mooseError("MultiAppSeedTransfer only supports transfers *to* multi-apps");
  if (!from_problem.subappsNeedReseed())
    return;

  for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i))
      transfer(_multi_app->appProblemBase(i), from_problem);
  _console << "Finished MultiAppSeedTransfer " << name() << std::endl;
}
