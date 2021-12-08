//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMatAndKernel.h"
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

registerMooseAction("MooseTestApp", AddMatAndKernel, "add_kernel");

registerMooseAction("MooseTestApp", AddMatAndKernel, "add_material");

registerMooseAction("MooseTestApp", AddMatAndKernel, "add_variable");

InputParameters
AddMatAndKernel::validParams()
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
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) that this object will be applied");
  return params;
}

AddMatAndKernel::AddMatAndKernel(const InputParameters & params) : Action(params) {}

void
AddMatAndKernel::act()
{
  std::string var_name = "var1";
  if (_current_task == "add_variable")
  {
    auto fe_type = AddVariableAction::feType(_pars);
    auto type = AddVariableAction::variableType(fe_type);
    auto var_params = _factory.getValidParams(type);

    var_params.set<MooseEnum>("family") = getParam<MooseEnum>("family");
    var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
    if (isParamValid("block"))
      var_params.set<std::vector<SubdomainName>>("block") =
          getParam<std::vector<SubdomainName>>("block");

    _problem->addVariable(type, var_name, var_params);
  }
  else if (_current_task == "add_kernel")
  {
    InputParameters params = _factory.getValidParams("MatDiffusionTest");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<MaterialPropertyName>("prop_name") = "prop1";
    _problem->addKernel("MatDiffusionTest", "kern1", params);
  }
  else if (_current_task == "add_material")
  {
    InputParameters params = _factory.getValidParams("GenericConstantMaterial");
    params.set<std::vector<std::string>>("prop_names") = {"prop1"};
    params.set<std::vector<Real>>("prop_values") = {42};
    _problem->addMaterial("GenericConstantMaterial", "mat1", params);
  }
}
