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
  std::vector<BoundaryName> everywhere(1);
  everywhere[0] = "ANY_BOUNDARY_ID";
  params.addParam<std::vector<BoundaryName> >("boundary", everywhere, "boundary ID or name where the normals will be computed");
  params.addParam<BoundaryName>("corner_boundary", "boundary ID or name with nodes at 'corners'");

  return params;
}

AddNodalNormalsAction::AddNodalNormalsAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _boundary(getParam<std::vector<BoundaryName> >("boundary"))
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

  bool has_corners = isParamValid("corner_boundary");
  if (has_corners)
    _corner_boundary = getParam<BoundaryName>("corner_boundary");

  {
    InputParameters pars = _factory.getValidParams("NodalNormalsPreprocessor");
    pars.set<MooseEnum>("execute_on") = execute_options;
    if (has_corners)
      pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
    _problem->addUserObject("NodalNormalsPreprocessor", "nodal_normals_preprocessor", pars);
  }
  if (has_corners)
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsCorner");
    pars.set<std::vector<BoundaryName> >("boundary") = _boundary;
    pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
    _problem->addUserObject("NodalNormalsCorner", "nodal_normals_corner", pars);
  }
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsEvaluator");
    pars.set<MooseEnum>("execute_on") = execute_options;
    pars.set<std::vector<BoundaryName> >("boundary") = _boundary;
    std::vector<SubdomainName> block_everywhere(1, "ANY_BLOCK_ID");
    pars.set<std::vector<SubdomainName> >("block") = block_everywhere;
    _problem->addUserObject("NodalNormalsEvaluator", "nodal_normals_preprocessor", pars);
  }
}
