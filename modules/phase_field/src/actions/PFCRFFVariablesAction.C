//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCRFFVariablesAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", PFCRFFVariablesAction, "add_variable");

InputParameters
PFCRFFVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  MooseEnum familyEnum = AddVariableAction::getNonlinearVariableFamilies();
  params.addParam<MooseEnum>(
      "family",
      familyEnum,
      "Specifies the family of FE shape functions to use for the L variables");
  MooseEnum orderEnum = AddVariableAction::getNonlinearVariableOrders();
  params.addParam<MooseEnum>(
      "order",
      orderEnum,
      "Specifies the order of the FE shape function to use for the L variables");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to the L variables");
  params.addRequiredParam<unsigned int>(
      "num_L", "specifies the number of complex L variables will be solved for");
  params.addRequiredParam<std::string>("L_name_base", "Base name for the complex L variables");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the variables and kernels");
  return params;
}

PFCRFFVariablesAction::PFCRFFVariablesAction(const InputParameters & params)
  : Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base"))
{
}

void
PFCRFFVariablesAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PFCRFFVariablesAction Object\n";
  Moose::err << "VariableBase: " << _L_name_base << "\torder: " << getParam<MooseEnum>("order")
             << "\tfamily: " << getParam<MooseEnum>("family") << std::endl;
#endif

  auto fe_type = AddVariableAction::feType(_pars);
  auto type = AddVariableAction::variableType(fe_type);
  auto var_params = _factory.getValidParams(type);

  var_params.applySpecificParameters(_pars, {"family", "order", "block"});
  var_params.set<std::vector<Real>>("scaling") = {getParam<Real>("scaling")};

  // Loop through the number of L variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base + Moose::stringify(l);

    // Create real L variable
    const std::string real_name = L_name + "_real";
    _problem->addVariable(type, real_name, var_params);

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name + "_imag";
      _problem->addVariable(type, imag_name, var_params);
    }
  }
}
