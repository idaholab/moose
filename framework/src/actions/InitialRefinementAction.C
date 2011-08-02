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
  if (level)
  {
    Moose::setup_perf_log.push("Uniformly Refine Mesh","Setup");
    // uniformly refine mesh
    _parser_handle._mesh->uniformlyRefine(level);
    _parser_handle._mesh->meshChanged();

    // Setup the displaced Mesh the same way
    if (_parser_handle._displaced_mesh)
    {
      _parser_handle._mesh->uniformlyRefine(level);
      _parser_handle._mesh->meshChanged();
    }
    
    Moose::setup_perf_log.pop("Uniformly Refine Mesh","Setup");

  }
#endif //LIBMESH_ENABLE_AMR

      std::cout << "Inside InitialRefinement Action *************\n";
}
