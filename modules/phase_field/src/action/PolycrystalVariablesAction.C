/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PolycrystalVariablesAction.h"
#include "Factory.h"
#include "FEProblem.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<PolycrystalVariablesAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription("Set up order parameter variables for a polycrystal sample");
  params.addParam<std::string>(
      "family", "LAGRANGE", "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::string>(
      "order", "FIRST", "Specifies the order of the FE shape function to use for this variable");
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addRequiredParam<unsigned int>("op_num",
                                        "specifies the number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  return params;
}

PolycrystalVariablesAction::PolycrystalVariablesAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalVariablesAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the PolycrystalVariablesAction Object\n"
             << "VariableBase: " << _var_name_base << "\torder: " << getParam<std::string>("order")
             << "\tfamily: " << getParam<std::string>("family") << std::endl;
#endif

  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Create variable names
    std::string var_name = _var_name_base;
    std::stringstream out;
    out << op;
    var_name.append(out.str());

    _problem->addVariable(
        var_name,
        FEType(Utility::string_to_enum<Order>(getParam<std::string>("order")),
               Utility::string_to_enum<FEFamily>(getParam<std::string>("family"))),
        getParam<Real>("scaling"));
  }
}
