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
#include "Parser.h"
#include "MooseMesh.h"
#include "FEProblem.h"

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
  mooseAssert(_parser_handle._mesh != NULL, "Mesh not setup");

  unsigned int level = getParam<unsigned int>("uniform_refine");
  if (level > 0)
  {
    Moose::setup_perf_log.push("Uniformly Refine Mesh","Setup");
    _parser_handle._problem->adaptivity().uniformRefine(level);
    Moose::setup_perf_log.pop("Uniformly Refine Mesh","Setup");

    // initial condition was set by EquationsSystems::init() (which already happened)
    // the mesh has changed and we need to set the initial condition again (because it
    // was actually interpolated during the refining process)
    Moose::setup_perf_log.push("Reproject solution","Setup");
    _parser_handle._problem->projectSolution();
    Moose::setup_perf_log.pop("Reproject solution","Setup");
  }
#endif //LIBMESH_ENABLE_AMR
}
