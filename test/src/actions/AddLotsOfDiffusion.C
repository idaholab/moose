//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLotsOfDiffusion.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "DirichletBC.h"
#include "AddVariableAction.h"

#include <sstream>
#include <stdexcept>

#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_variable");

registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_kernel");

registerMooseAction("MooseTestApp", AddLotsOfDiffusion, "add_bc");

InputParameters
AddLotsOfDiffusion::validParams()
{
  InputParameters params = Action::validParams();

  MooseEnum order(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH", "FIRST", true);
  params.addParam<MooseEnum>("order",
                             order,
                             "Order of the FE shape function to use for this variable (additional "
                             "orders not listed here are allowed, depending on the family).");

  MooseEnum family("LAGRANGE MONOMIAL HERMITE SCALAR HIERARCHIC CLOUGH XYZ SZABAB BERNSTEIN "
                   "L2_LAGRANGE L2_HIERARCHIC NEDELEC_ONE LAGRANGE_VEC",
                   "LAGRANGE");
  params.addParam<MooseEnum>(
      "family", family, "Specifies the family of FE shape functions to use for this variable.");

  params.addRequiredParam<unsigned int>("number", "The number of variables to add");

  return params;
}

AddLotsOfDiffusion::AddLotsOfDiffusion(const InputParameters & params) : Action(params) {}

void
AddLotsOfDiffusion::act()
{
  unsigned int number = getParam<unsigned int>("number");

  if (_current_task == "add_variable")
  {
    auto fe_type = AddVariableAction::feType(_pars);
    auto type = AddVariableAction::determineType(fe_type, 1, false);
    auto var_params = _factory.getValidParams(type);

    var_params.set<MooseEnum>("family") = getParam<MooseEnum>("family");
    var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");

    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);
      _problem->addVariable(type, var_name, var_params);
    }
  }
  else if (_current_task == "add_kernel")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);

      InputParameters params = _factory.getValidParams("Diffusion");
      params.set<NonlinearVariableName>("variable") = var_name;
      _problem->addKernel("Diffusion", var_name, params);
    }
  }
  else if (_current_task == "add_bc")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);

      InputParameters params = _factory.getValidParams("DirichletBC");
      params.set<NonlinearVariableName>("variable") = var_name;
      params.set<std::vector<BoundaryName>>("boundary").push_back("left");
      params.set<Real>("value") = 0;

      _problem->addBoundaryCondition("DirichletBC", var_name + "_left", params);

      params.set<std::vector<BoundaryName>>("boundary")[0] = "right";
      params.set<Real>("value") = 1;

      _problem->addBoundaryCondition("DirichletBC", var_name + "_right", params);
    }
  }
}
