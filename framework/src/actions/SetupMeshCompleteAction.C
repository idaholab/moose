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

#include "SetupMeshCompleteAction.h"
#include "MooseMesh.h"
#include "Moose.h"

template<>
InputParameters validParams<SetupMeshCompleteAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupMeshCompleteAction::SetupMeshCompleteAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupMeshCompleteAction::completeSetup(MooseMesh *mesh)
{
  Moose::setup_perf_log.push("Prepare Mesh","Setup");
  mesh->prepare();
  Moose::setup_perf_log.pop("Prepare Mesh","Setup");

  Moose::setup_perf_log.push("Initial meshChanged()","Setup");
  mesh->meshChanged();
  Moose::setup_perf_log.pop("Initial meshChanged()","Setup");
}


void
SetupMeshCompleteAction::act()
{
  if (_mesh)
  {
    completeSetup(_mesh);
    _mesh->printInfo();
  }
  else
    mooseError("No mesh file was supplied and no generation block was provided");

  if (_displaced_mesh)
    completeSetup(_displaced_mesh);
}
