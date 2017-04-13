/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// Standard includes
#include <sstream>
#include <stdexcept>

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "MooseEigenSystem.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"

// class static initialization
const Real AddVariableAction::_abs_zero_tol = 1e-12;

template <>
InputParameters
validParams<AddVariableAction>()
{
  // Get MooseEnums for the possible order/family options for this variable
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  // Define the general input options
  InputParameters params = validParams<Action>();
  params += validParams<OutputInterface>();
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<Real>("initial_condition", "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this variable lives");
  params.addParam<bool>("eigen", false, "True to make this variable an eigen variable");

  // Advanced input options
  params.addParam<Real>("scaling", 1.0, "Specifies a scaling factor to apply to this variable");
  params.addParamNamesToGroup("scaling eigen", "Advanced");

  return params;
}

AddVariableAction::AddVariableAction(InputParameters params)
  : Action(params),
    OutputInterface(params, false),
    _fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
             Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family"))),
    _scalar_var(_fe_type.family == SCALAR)
{
}

MooseEnum
AddVariableAction::getNonlinearVariableFamilies()
{
  return MooseEnum("LAGRANGE MONOMIAL HERMITE SCALAR HIERARCHIC CLOUGH XYZ SZABAB BERNSTEIN "
                   "L2_LAGRANGE L2_HIERARCHIC",
                   "LAGRANGE");
}

MooseEnum
AddVariableAction::getNonlinearVariableOrders()
{
  return MooseEnum("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST", true);
}

void
AddVariableAction::act()
{
  // Get necessary data for creating a variable
  std::string var_name = name();
  addVariable(var_name);

  // Set the initial condition
  if (isParamValid("initial_condition"))
    createInitialConditionAction();
}

void
AddVariableAction::createInitialConditionAction()
{
  // Variable name
  std::string var_name = name();

  // Create the object name
  std::string long_name("");
  long_name += var_name;
  long_name += "_moose";

  // Set the parameters for the action
  InputParameters action_params = _action_factory.getValidParams("AddOutputAction");
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  if (_scalar_var)
    action_params.set<std::string>("type") = "ScalarConstantIC";
  else
    action_params.set<std::string>("type") = "ConstantIC";

  // Create the action
  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create("AddInitialConditionAction", long_name, action_params));

  // Set the required parameters for the object to be created
  action->getObjectParams().set<VariableName>("variable") = var_name;
  action->getObjectParams().set<Real>("value") = getParam<Real>("initial_condition");

  // Store the action in the ActionWarehouse
  _awh.addActionBlock(action);
}

void
AddVariableAction::addVariable(std::string & var_name)
{
  std::set<SubdomainID> blocks = getSubdomainIDs();
  Real scale_factor = isParamValid("scaling") ? getParam<Real>("scaling") : 1;

  // Scalar variable
  if (_scalar_var)
    _problem->addScalarVariable(var_name, _fe_type.order, scale_factor);

  // Block restricted variable
  else if (blocks.empty())
    _problem->addVariable(var_name, _fe_type, scale_factor);

  // Non-block restricted variable
  else
    _problem->addVariable(var_name, _fe_type, scale_factor, &blocks);

  if (getParam<bool>("eigen"))
  {
    MooseEigenSystem & esys(static_cast<MooseEigenSystem &>(_problem->getNonlinearSystemBase()));
    esys.markEigenVariable(var_name);
  }
}

std::set<SubdomainID>
AddVariableAction::getSubdomainIDs()
{
  // Extract and return the block ids supplied in the input
  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param = getParam<std::vector<SubdomainName>>("block");
  for (const auto & subdomain_name : block_param)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(subdomain_name);
    blocks.insert(blk_id);
  }
  return blocks;
}
