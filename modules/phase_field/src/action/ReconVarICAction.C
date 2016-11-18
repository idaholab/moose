/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ReconVarICAction.h"
#include "Factory.h"
#include "Parser.h"
#include "FEProblemBase.h"
#include "Conversion.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"

template<>
InputParameters validParams<ReconVarICAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredParam<unsigned int>("op_num", "Specifies the number of order paraameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addParam<unsigned int>("phase", "EBSD phase number to be assigned to this grain");
  params.addParam<bool>("advanced_op_assignment", false, "Enable advanced grain to op assignment (avoid invalid graph coloring)");
  return params;
}

ReconVarICAction::ReconVarICAction(const InputParameters & params) :
    Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
ReconVarICAction::act()
{
#ifdef DEBUG
  Moose::err << "Inside the ReconVarICAction Object\n";
#endif

  if (_current_task == "add_ic")
  {
    // Set initial condition for each order parameter
    for (unsigned int op = 0; op < _op_num; ++op)
    {
      // Create variable names
      std::string var_name = _var_name_base;
      std::stringstream out;
      out << op;
      var_name.append(out.str());

      {
        // Define parameters for ReconVarIC
        InputParameters poly_params = _factory.getValidParams("ReconVarIC");
        poly_params.set<VariableName>("variable") = var_name;
        poly_params.set<unsigned int>("op_index") = op;
        poly_params.set<unsigned int>("op_num") = _op_num;
        poly_params.set<UserObjectName>("ebsd_reader") = getParam<UserObjectName>("ebsd_reader");
        poly_params.set<bool>("advanced_op_assignment") = getParam<bool>("advanced_op_assignment");
        if (isParamValid("phase"))
          poly_params.set<unsigned int>("phase") = getParam<unsigned int>("phase");

        // Add initial condition
        _problem->addInitialCondition("ReconVarIC", "Initialize_op_" + Moose::stringify(op), poly_params);
      }
    }

    // Add the elemental op
    InputParameters poly_params = _factory.getValidParams("ReconVarIC");
    poly_params.set<VariableName>("variable") = std::string(_var_name_base + "_op");
    poly_params.set<unsigned int>("op_index") = 0; // Unused
    poly_params.set<unsigned int>("op_num") = _op_num;
    poly_params.set<bool>("all_op_elemental") = true;
    poly_params.set<UserObjectName>("ebsd_reader") = getParam<UserObjectName>("ebsd_reader");
    poly_params.set<bool>("advanced_op_assignment") = getParam<bool>("advanced_op_assignment");
    if (isParamValid("phase"))
      poly_params.set<unsigned int>("phase") = getParam<unsigned int>("phase");

    // Add initial condition
    _problem->addInitialCondition("ReconVarIC", "Initialize_elem_op", poly_params);
  }
  else if (_current_task == "add_aux_variable")
  {
    Order order = CONSTANT;
    FEFamily family = MONOMIAL;
    FEType fe_type(order, family);

    _problem->addAuxVariable(std::string(_var_name_base + "_op"), fe_type);
  }
}
