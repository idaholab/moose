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

#include "InitialRefinementAction.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "CommandLine.h"

template<>
InputParameters validParams<InitialRefinementAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<unsigned int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh");

  return params;
}

InitialRefinementAction::InitialRefinementAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
InitialRefinementAction::act()
{
#ifdef LIBMESH_ENABLE_AMR
  mooseAssert(_mesh != NULL, "Mesh not setup");

  unsigned int level = getParam<unsigned int>("uniform_refine");

  std::string arg;
  if (Moose::app->commandLine().search("REFINE", &arg))
  {
    unsigned int auto_refine_levels = 0;
    std::stringstream ss(arg);
    ss >> auto_refine_levels;
    level += auto_refine_levels;
  }

  _problem->setUniformRefineLevel(level);
#endif //LIBMESH_ENABLE_AMR
}
