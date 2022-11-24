//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddNodalNormalsAction.h"
#include "FEProblem.h"
#include "Factory.h"

#include "libmesh/fe.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("MooseApp", AddNodalNormalsAction, "add_aux_variable");

registerMooseAction("MooseApp", AddNodalNormalsAction, "add_postprocessor");

registerMooseAction("MooseApp", AddNodalNormalsAction, "add_user_object");

InputParameters
AddNodalNormalsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates Auxiliary variables and objects for computing the outward "
                             "facing normal from a node.");

  // Initialize the 'boundary' input option to default to any boundary
  std::vector<BoundaryName> everywhere(1, "ANY_BOUNDARY_ID");
  params.addParam<std::vector<BoundaryName>>(
      "boundary", everywhere, "The boundary ID or name where the normals will be computed");
  params.addParam<BoundaryName>("corner_boundary", "boundary ID or name with nodes at 'corners'");
  MooseEnum orders("FIRST SECOND", "FIRST");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of variables that hold the "
                             "nodal normals. Needs to match the order of the "
                             "mesh");

  return params;
}

AddNodalNormalsAction::AddNodalNormalsAction(const InputParameters & parameters)
  : Action(parameters),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _has_corners(isParamValid("corner_boundary")),
    _corner_boundary(_has_corners ? getParam<BoundaryName>("corner_boundary") : BoundaryName())
{
}

void
AddNodalNormalsAction::act()
{
  // Set the order from the input
  auto enum_order = getParam<MooseEnum>("order");
  Order order = Utility::string_to_enum<Order>(enum_order);
  FEFamily family = LAGRANGE;

  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "LAGRANGE";
  var_params.set<MooseEnum>("order") = enum_order;

  // Add 3 aux variables for each component of the normal
  if (_current_task == "add_aux_variable")
  {
    _problem->addAuxVariable("MooseVariable", "nodal_normal_x", var_params);
    _problem->addAuxVariable("MooseVariable", "nodal_normal_y", var_params);
    _problem->addAuxVariable("MooseVariable", "nodal_normal_z", var_params);
  }

  // Set the execute options
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};

  // Create the NodalNormalsPreprocessor UserObject
  if (_current_task == "add_postprocessor")
  {
    InputParameters pars = _factory.getValidParams("NodalNormalsPreprocessor");
    pars.set<Order>("fe_order") = order;
    pars.set<FEFamily>("fe_family") = family;
    pars.set<ExecFlagEnum>("execute_on") = execute_options;
    pars.set<std::vector<BoundaryName>>("surface_boundary") = _boundary;

    if (_has_corners)
      pars.set<BoundaryName>("corner_boundary") = _corner_boundary;

    _problem->addUserObject("NodalNormalsPreprocessor", "nodal_normals_preprocessor", pars);
  }

  if (_current_task == "add_user_object")
  {
    /// Create the NodalNormalsCorner UserObject (only if corner boundary is given)
    if (_has_corners)
    {
      InputParameters pars = _factory.getValidParams("NodalNormalsCorner");
      pars.set<ExecFlagEnum>("execute_on") = execute_options;
      pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
      pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
      _problem->addUserObject("NodalNormalsCorner", "nodal_normals_corner", pars);
    }

    /// Create the NodalNormalsEvaluator UserObject
    {
      InputParameters pars = _factory.getValidParams("NodalNormalsEvaluator");
      pars.set<ExecFlagEnum>("execute_on") = execute_options;
      pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
      _problem->addUserObject("NodalNormalsEvaluator", "nodal_normals_evaluator", pars);
    }
  }
}
