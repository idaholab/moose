/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFCRFFVariablesAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<PFCRFFVariablesAction>()
{
  InputParameters params = validParams<Action>();
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

  // Loop through the number of L variables
  for (unsigned int l = 0; l < _num_L; ++l)
  {
    // Create L base name
    std::string L_name = _L_name_base + Moose::stringify(l);

    // Create real L variable
    const std::string real_name = L_name + "_real";
    _problem->addVariable(real_name,
                          FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
                          getParam<Real>("scaling"));

    if (l > 0)
    {
      // Create imaginary L variable IF l > 0
      std::string imag_name = L_name + "_imag";
      _problem->addVariable(
          imag_name,
          FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                 Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
          getParam<Real>("scaling"));
    }
  }
}
