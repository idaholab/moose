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

#include "InitDisplacedProblemAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

template<>
InputParameters validParams<InitDisplacedProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<std::string> >("displacements", "The variables corresponding to the x y z displacements of the mesh.  If this is provided then the displacements will be taken into account during the computation.");

  return params;
}

InitDisplacedProblemAction::InitDisplacedProblemAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

InitDisplacedProblemAction::~InitDisplacedProblemAction()
{
}

void
InitDisplacedProblemAction::act()
{
  if (isParamValid("displacements"))
  {
    InputParameters params = validParams<DisplacedProblem>();
    params.set<std::vector<std::string> >("displacements") = getParam<std::vector<std::string> >("displacements");
    _parser_handle._problem->initDisplacedProblem(_parser_handle._displaced_mesh, params);
  }
}
