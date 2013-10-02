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

#include "libmesh/fe.h"
#include "libmesh/string_to_enum.h"


template<>
InputParameters validParams<AddNodalNormalsAction>()
{
  InputParameters params = validParams<Action>();

  // Initialize the 'boundary' input option to default to any boundary
  std::vector<BoundaryName> everywhere(1, "ANY_BOUNDARY_ID");
  params.addParam<std::vector<BoundaryName> >("boundary", everywhere, "The boundary ID or name where the normals will be computed");
  params.addParam<BoundaryName>("corner_boundary", "boundary ID or name with nodes at 'corners'");
MooseEnum orders("FIRST, SECOND", "FIRST");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of variables that hold the nodal normals. Needs to match the order of the mesh");

  return params;
}

AddNodalNormalsAction::AddNodalNormalsAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _boundary(getParam<std::vector<BoundaryName> >("boundary")),
    _has_corners(isParamValid("corner_boundary")),
    _corner_boundary(_has_corners ? getParam<BoundaryName>("corner_boundary") : BoundaryName())
{
}

AddNodalNormalsAction::~AddNodalNormalsAction()
{
}

void
AddNodalNormalsAction::act()
{
  // Set the order from the input
  Order order = Utility::string_to_enum<Order>(getParam<MooseEnum>("order"));
  FEFamily family = LAGRANGE;
  FEType fe_type(order, family);

  // Add 3 aux variables for each component of the normal
  _problem->addAuxVariable("nodal_normal_x", fe_type);
  _problem->addAuxVariable("nodal_normal_y", fe_type);
  _problem->addAuxVariable("nodal_normal_z", fe_type);

  // Set the execute options
  MooseEnum execute_options(SetupInterface::getExecuteOptions());
  execute_options = "timestep_begin";

  // Create the NodalNormalsPreprocessor UserObject
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsPreprocessor");
    pars.set<Order>("fe_order") = order;
    pars.set<FEFamily>("fe_family") = family;
    pars.set<MooseEnum>("execute_on") = execute_options;
    pars.set<std::vector<BoundaryName> >("boundary") = _boundary;

    if (_has_corners)
      pars.set<BoundaryName>("corner_boundary") = _corner_boundary;

    _problem->addUserObject("NodalNormalsPreprocessor", "nodal_normals_preprocessor", pars);
  }

  /// Create the NodalNormalsCorner UserObject (only if corner boundary is given)
  if (_has_corners)
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsCorner");
    pars.set<MooseEnum>("execute_on") = execute_options;
    pars.set<std::vector<BoundaryName> >("boundary") = _boundary;
    pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
    _problem->addUserObject("NodalNormalsCorner", "nodal_normals_corner", pars);
  }

  /// Create the NodalNormalsEvaluator UserObject
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsEvaluator");
    pars.set<MooseEnum>("execute_on") = execute_options;
    pars.set<std::vector<BoundaryName> >("boundary") = _boundary;
    _problem->addUserObject("NodalNormalsEvaluator", "nodal_normals_evaluator", pars);
  }
}
