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

#include "AddNodalNormalsAction.h"
#include "FEProblem.h"
#include "Factory.h"


template<>
InputParameters validParams<AddNodalNormalsAction>()
{
  InputParameters params = validParams<Action>();

  return params;
}

AddNodalNormalsAction::AddNodalNormalsAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

AddNodalNormalsAction::~AddNodalNormalsAction()
{
}

void
AddNodalNormalsAction::act()
{
  AuxiliarySystem & aux_sys = _problem->getAuxiliarySystem();
  // if there is not variable in aux system, we have to add one dummy variable, so our vectors have non-zero length
  if (aux_sys.nVariables() == 0)
    aux_sys.addVariable("dummy", FEType(FIRST, LAGRANGE), 1.);

  if (LIBMESH_DIM >= 1)
    aux_sys.addVector("nx", false, GHOSTED);
  if (LIBMESH_DIM >= 2)
    aux_sys.addVector("ny", false, GHOSTED);
  if (LIBMESH_DIM >= 3)
    aux_sys.addVector("nz", false, GHOSTED);

  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep";

  {
    InputParameters pars = _factory.getValidParams("NodalNormalsPreprocessor");
    pars.set<MooseEnum>("execute_on") = execute_options;
    _problem->addUserObject("NodalNormalsPreprocessor", "nodal_normals_preprocessor", pars);
  }
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsEvaluator");
    pars.set<MooseEnum>("execute_on") = execute_options;
    std::vector<BoundaryName> everywhere(1, "ANY_BOUNDARY_ID");
    pars.set<std::vector<BoundaryName> >("boundary") = everywhere;
    std::vector<SubdomainName> block_everywhere(1, "ANY_BLOCK_ID");
    pars.set<std::vector<SubdomainName> >("block") = block_everywhere;
    _problem->addUserObject("NodalNormalsEvaluator", "nodal_normals_preprocessor", pars);
  }
}
