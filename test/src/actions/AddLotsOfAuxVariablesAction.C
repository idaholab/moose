//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddLotsOfAuxVariablesAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "MooseMesh.h"

#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

// System includes
#include <sstream>
#include <stdexcept>

// class static initialization
const Real AddLotsOfAuxVariablesAction::_abs_zero_tol = 1e-12;

registerMooseAction("MooseTestApp", AddLotsOfAuxVariablesAction, "meta_action");

InputParameters
AddLotsOfAuxVariablesAction::validParams()
{
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  InputParameters params = Action::validParams();
  params.addRequiredParam<unsigned int>("number", "The number of variables to add");
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>(
      "order", orders, "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>(
      "initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this variable lives");

  return params;
}

AddLotsOfAuxVariablesAction::AddLotsOfAuxVariablesAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddLotsOfAuxVariablesAction::act()
{
  auto fe_type = AddVariableAction::feType(_pars);
  auto type = AddVariableAction::variableType(fe_type);
  auto var_params = _factory.getValidParams(type);

  var_params.set<MooseEnum>("family") = getParam<MooseEnum>("family");
  var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  if (isParamValid("block"))
    var_params.set<std::vector<SubdomainName>>("block") =
        getParam<std::vector<SubdomainName>>("block");

  bool scalar_var = fe_type.family == SCALAR;

  unsigned int number = getParam<unsigned int>("number");
  for (unsigned int cur_num = 0; cur_num < number; cur_num++)
  {
    std::string var_name = name() + Moose::stringify(cur_num);

    _problem->addAuxVariable(type, var_name, var_params);

    // Set initial condition
    Real initial = getParam<Real>("initial_condition");
    if (initial > _abs_zero_tol || initial < -_abs_zero_tol)
    {
      if (scalar_var)
      {
        // built a ScalarConstantIC object
        InputParameters params = _factory.getValidParams("ScalarConstantIC");
        params.set<VariableName>("variable") = var_name;
        params.set<Real>("value") = initial;
        _problem->addInitialCondition("ScalarConstantIC", "ic", params);
      }
      else
      {
        // built a ConstantIC object
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = var_name;
        params.set<Real>("value") = initial;
        _problem->addInitialCondition("ConstantIC", "ic", params);
      }
    }
  }
}
