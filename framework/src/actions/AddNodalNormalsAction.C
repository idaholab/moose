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

template <>
InputParameters
validParams<AddNodalNormalsAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) that this object will be applied");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The boundary ID or name where the normals will be computed");
  params.addParam<BoundaryName>("corner_boundary", "boundary ID or name with nodes at 'corners'");
  // By default, compute the nodal normals just once before we start solving
  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL};
  params.addParam<ExecFlagEnum>("execute_on",
                                exec_enum,
                                "Set to (nonlinear|linear|timestep_end|timestep_begin|custom) "
                                "to execute only at that moment");
  MooseEnum orders("FIRST SECOND", "FIRST");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of variables that hold the nodal normals. Needs "
                             "to match the order of the mesh");
  return params;
}

AddNodalNormalsAction::AddNodalNormalsAction(InputParameters parameters)
  : Action(parameters),
    _block(getParam<std::vector<SubdomainName>>("block")),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _has_corners(isParamValid("corner_boundary")),
    _corner_boundary(_has_corners ? getParam<BoundaryName>("corner_boundary") : BoundaryName()),
    _execute_options(getParam<ExecFlagEnum>("execute_on"))
{
}

void
AddNodalNormalsAction::act()
{
  if (_current_task == "add_user_object")
  {
    Order order = Utility::string_to_enum<Order>(getParam<MooseEnum>("order"));

    {
      std::string class_name = "NodalNormalsUserObject";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
      pars.set<ExecFlagEnum>("execute_on") = _execute_options;
      _problem->addUserObject(class_name, name(), pars);
    }

    {
      std::string class_name = "NodalNormalsBoundaryNodes";
      InputParameters pars = _factory.getValidParams(class_name);
      if (isParamValid("block"))
        pars.set<std::vector<SubdomainName>>("block") = _block;
      pars.set<Order>("fe_order") = order;
      pars.set<UserObjectName>("nodal_normals_uo") = name();
      if (_has_corners)
        pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
      pars.set<ExecFlagEnum>("execute_on") = _execute_options;
      _problem->addUserObject(class_name, name() + ":bnd_nodes", pars);
    }

    if (_has_corners)
    {
      std::string class_name = "NodalNormalsCorner";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
      pars.set<UserObjectName>("nodal_normals_uo") = name();
      pars.set<BoundaryName>("corner_boundary") = _corner_boundary;
      pars.set<ExecFlagEnum>("execute_on") = _execute_options;
      _problem->addUserObject(class_name, name() + ":corners", pars);
    }
  }
}
