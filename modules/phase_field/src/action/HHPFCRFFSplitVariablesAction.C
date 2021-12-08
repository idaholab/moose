//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HHPFCRFFSplitVariablesAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("PhaseFieldApp", HHPFCRFFSplitVariablesAction, "add_variable");

InputParameters
HHPFCRFFSplitVariablesAction::validParams()
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

  return params;
}

HHPFCRFFSplitVariablesAction::HHPFCRFFSplitVariablesAction(const InputParameters & params)
  : Action(params),
    _num_L(getParam<unsigned int>("num_L")),
    _L_name_base(getParam<std::string>("L_name_base"))
{
}

void
HHPFCRFFSplitVariablesAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the HHPFCRFFSplitVariablesAction Object\n";
  Moose::err << "VariableBase: " << _L_name_base << "\torder: " << getParam<MooseEnum>("order")
             << "\tfamily: " << getParam<MooseEnum>("family") << std::endl;
#endif

  const auto variable_type = AddVariableAction::variableType(
      FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))));
  auto variable_params = _factory.getValidParams(variable_type);
  variable_params.set<MooseEnum>("family") = getParam<MooseEnum>("family");
  variable_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  variable_params.set<Real>("scaling") = getParam<Real>("scaling");

  // Loop through the number of L variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base + Moose::stringify(l);

    // Create real L variable
    std::string real_name = L_name + "_real";

    InputParameters real_params = variable_params;
    _problem->addAuxVariable(variable_type, real_name, real_params);

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name + "_imag";
      InputParameters imag_params = variable_params;
      _problem->addAuxVariable(variable_type, imag_name, imag_params);
    }
  }
}
