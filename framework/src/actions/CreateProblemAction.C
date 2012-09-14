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

#include "CreateProblemAction.h"
#include "ProblemFactory.h"
#include "FEProblem.h"

template<>
InputParameters validParams<CreateProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("type", "FEProblem", "Type of the problem to build");
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  params.addParam<std::vector<SubdomainName> >("block", "Block IDs for the coordinate systems");
  params.addParam<std::vector<std::string> >("coord_type", "Type of the coordinate system per block param");

  params.addParam<bool>("fe_cache", false, "Whether or not to turn on the finite element shape function caching system.  This can increase speed with an associated memory cost.");
  return params;
}


CreateProblemAction::CreateProblemAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _type(getParam<std::string>("type")),
    _problem_name(getParam<std::string>("name")),
    _blocks(getParam<std::vector<SubdomainName> >("block")),
    _coord_sys(getParam<std::vector<std::string> >("coord_type")),
    _fe_cache(getParam<bool>("fe_cache"))
{
}

void
CreateProblemAction::act()
{
  if (_mesh != NULL)
  {
    // build the problem only if we have mesh
    {
      InputParameters params = validParams<FEProblem>();
      params.set<MooseMesh *>("mesh") = _mesh;
      _problem = dynamic_cast<FEProblem *>(ProblemFactory::instance()->create(_type, _problem_name, params));
      if (_problem == NULL)
        mooseError("Problem has to be of a FEProblem type");
    }
    // set up the problem
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->useFECache(_fe_cache);
  }
}
