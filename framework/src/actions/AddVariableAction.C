//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "MooseEigenSystem.h"
#include "MooseObjectAction.h"
#include "MooseMesh.h"
#include "CopyNodalVarsAction.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/fe_interface.h"

using namespace libMesh;

registerMooseAction("MooseApp", AddVariableAction, "add_variable");

InputParameters
AddVariableAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Add a non-linear variable to the simulation.");

  // The user may specify a type in the Variables block, but if they don't we'll just use all the
  // parameters available from MooseVariableBase
  params.set<std::string>("type") = "MooseVariableBase";

  // The below is for backwards compatibility
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<std::vector<Real>>("scaling",
                                     "Specifies a scaling factor to apply to this variable");
  params.addParam<std::vector<Real>>("initial_condition",
                                     "Specifies a constant initial condition for this variable");
  params.transferParam<std::string>(CopyNodalVarsAction::validParams(), "initial_from_file_var");
  return params;
}

AddVariableAction::AddVariableAction(const InputParameters & params)
  : MooseObjectAction(params),
    _fe_type(feType(params)),
    _scalar_var(_fe_type.family == SCALAR),
    _fv_var(false),
    _components(1)
{
}

MooseEnum
AddVariableAction::getNonlinearVariableFamilies()
{
  return MooseEnum("LAGRANGE MONOMIAL HERMITE SCALAR HIERARCHIC CLOUGH XYZ SZABAB BERNSTEIN "
                   "L2_LAGRANGE L2_HIERARCHIC NEDELEC_ONE LAGRANGE_VEC MONOMIAL_VEC "
                   "RAVIART_THOMAS RATIONAL_BERNSTEIN SIDE_HIERARCHIC L2_HIERARCHIC_VEC "
                   "L2_LAGRANGE_VEC L2_RAVIART_THOMAS",
                   "LAGRANGE");
}

MooseEnum
AddVariableAction::getNonlinearVariableOrders()
{
  return MooseEnum("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST", true);
}

FEType
AddVariableAction::feType(const InputParameters & params)
{
  return {Utility::string_to_enum<Order>(params.get<MooseEnum>("order")),
          Utility::string_to_enum<FEFamily>(params.get<MooseEnum>("family"))};
}

void
AddVariableAction::init()
{
  _components = _moose_object_pars.get<unsigned int>("components");
  if (_components == 0)
    mooseError("There must be at least one variable component, but somehow 0 has been specified");

  // We have to do some sanity checks because of our work to maintain backwards compatibility.
  // `family`, `order`, and `scaling` are all parameters duplicated between this action and the
  // `MooseVariable*` object itself. Consequently during input file parsing, the params objects for
  // both the action and MooseVariable object can be populated with the exact same parameters.
  // However, some applications actually create their variables solely through creation and setting
  // of `AddVariableAction` parameters which means that the `MooseVariableBase*` params will never
  // be populated. So we should apply the parameters directly from the action. There should be no
  // case in which both params objects get set by the user and they have different values

  if (isParamSetByUser("family") && _moose_object_pars.isParamSetByUser("family") &&
      !getParam<MooseEnum>("family").compareCurrent(_moose_object_pars.get<MooseEnum>("family")))
    mooseError("Both the MooseVariable* and Add*VariableAction parameters objects have had the "
               "`family` parameter set, and they are different values: ",
               _moose_object_pars.get<MooseEnum>("family"),
               " and ",
               getParam<MooseEnum>("family"),
               " respectively. I don't know how you achieved this, but you need to rectify it.");

  if (isParamSetByUser("order") && _moose_object_pars.isParamSetByUser("order") &&
      !getParam<MooseEnum>("order").compareCurrent(_moose_object_pars.get<MooseEnum>("order")))
    mooseError("Both the MooseVariable* and Add*VariableAction parameters objects have had the "
               "`order` parameter set, and they are different values: ",
               _moose_object_pars.get<MooseEnum>("order"),
               " and ",
               getParam<MooseEnum>("order"),
               " respectively. I don't know how you achieved this, but you need to rectify it.");

  if (isParamSetByUser("scaling") && _moose_object_pars.isParamSetByUser("scaling") &&
      getParam<std::vector<Real>>("scaling") !=
          _moose_object_pars.get<std::vector<Real>>("scaling"))
    mooseError("Both the MooseVariable* and Add*VariableAction parameters objects have had the "
               "`scaling` parameter set, and they are different values. I don't know how you "
               "achieved this, but you need to rectify it.");

  if (isParamSetByUser("initial_condition") && isParamSetByUser("initial_from_file_var"))
    paramError("initial_condition",
               "Two initial conditions have been provided for the variable ",
               name(),
               " using the 'initial_condition' and 'initial_from_file_var' parameters. Please "
               "remove one of them.");

  _moose_object_pars.applySpecificParameters(_pars, {"order", "family", "scaling"});

  // Determine the MooseVariable type
  _fv_var = _moose_object_pars.get<bool>("fv");
  const auto is_array = _components > 1 || _moose_object_pars.get<bool>("array");
  if (_type == "MooseVariableBase")
    _type = variableType(_fe_type, _fv_var, is_array);
  if (_fv_var)
    _problem->needFV();

  // Need static_cast to resolve overloads
  _problem_add_var_method = static_cast<void (FEProblemBase::*)(
      const std::string &, const std::string &, InputParameters &)>(&FEProblemBase::addVariable);
}

void
AddVariableAction::act()
{
  // If we've been called that means that current_task == "add_variable"
  init();

  // Get necessary data for creating a variable
  const auto var_name = varName();
  addVariable(var_name);

  // Set the initial condition
  if (isParamValid("initial_condition"))
  {
    const auto & value = getParam<std::vector<Real>>("initial_condition");
    createInitialConditionAction(value);
  }
}

void
AddVariableAction::createInitialConditionAction(const std::vector<Real> & value)
{
  // Variable name
  const auto var_name = varName();

  // Create the object name
  std::string long_name("");
  long_name += var_name;
  long_name += "_moose";

  // Set the parameters for the action
  InputParameters action_params = _action_factory.getValidParams("AddOutputAction");
  action_params.set<ActionWarehouse *>("awh") = &_awh;

  // Associate all action and initial condition errors with "initial_condition"
  associateWithParameter("initial_condition", action_params);

  const auto fe_field_type = FEInterface::field_type(_fe_type);
  const bool is_vector = fe_field_type == TYPE_VECTOR;

  if (_scalar_var)
    action_params.set<std::string>("type") = "ScalarConstantIC";
  else if (_components == 1)
  {
    if (is_vector)
      action_params.set<std::string>("type") = "VectorConstantIC";
    else
    {
      if (_fv_var)
        action_params.set<std::string>("type") = "FVConstantIC";
      else
        action_params.set<std::string>("type") = "ConstantIC";
    }
  }
  else
  {
    action_params.set<std::string>("type") = "ArrayConstantIC";
    if (value.size() != _components)
      mooseError("Size of 'initial_condition' is not consistent");
  }

  // Create the action
  std::shared_ptr<MooseObjectAction> action;
  if (_fv_var)
    action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("AddFVInitialConditionAction", long_name, action_params));
  else
    action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("AddInitialConditionAction", long_name, action_params));

  // Set the required parameters for the object to be created
  action->getObjectParams().set<VariableName>("variable") = var_name;
  if (_components > 1)
  {
    RealEigenVector v(_components);
    for (unsigned int i = 0; i < _components; ++i)
      v(i) = value[i];
    action->getObjectParams().set<RealEigenVector>("value") = v;
  }
  else if (is_vector)
  {
    action->getObjectParams().set<Real>("x_value") = value[0];
    if (value.size() > 0)
      action->getObjectParams().set<Real>("y_value") = value[1];
    if (value.size() > 1)
      action->getObjectParams().set<Real>("z_value") = value[2];
  }
  else
    action->getObjectParams().set<Real>("value") = value[0];

  // Store the action in the ActionWarehouse
  _awh.addActionBlock(action);
}

std::string
AddVariableAction::determineType(const FEType & fe_type, unsigned int components, bool is_fv)
{
  ::mooseDeprecated("AddVariableAction::determineType() is deprecated. Use "
                    "AddVariableAction::variableType() instead.");
  return variableType(fe_type, is_fv, components > 1);
}

std::string
AddVariableAction::variableType(const FEType & fe_type, const bool is_fv, const bool is_array)
{
  if (is_fv)
    return "MooseVariableFVReal";

  const auto fe_field_type = FEInterface::field_type(fe_type);

  if (is_array)
  {
    if (fe_field_type == TYPE_VECTOR)
      ::mooseError("Vector finite element families do not currently have ArrayVariable support");
    else
      return "ArrayMooseVariable";
  }
  else if (fe_type == FEType(0, MONOMIAL))
    return "MooseVariableConstMonomial";
  else if (fe_type.family == SCALAR)
    return "MooseVariableScalar";
  else if (fe_field_type == TYPE_VECTOR)
    return "VectorMooseVariable";
  else
    return "MooseVariable";
}

void
AddVariableAction::addVariable(const std::string & var_name)
{
  // Compare sizes of scaling_factor and components for Array Variables
  const auto & scale_factor = _moose_object_pars.isParamValid("scaling")
                                  ? _moose_object_pars.get<std::vector<Real>>("scaling")
                                  : std::vector<Real>(_components, 1);
  if (scale_factor.size() != _components)
    mooseError("Size of 'scaling' is not consistent");

  _problem_add_var_method(*_problem, _type, var_name, _moose_object_pars);

  if (_moose_object_pars.get<bool>("eigen"))
  {
    // MooseEigenSystem will be eventually removed. NonlinearEigenSystem will be used intead.
    // It is legal for NonlinearEigenSystem to specify a variable as eigen in input file,
    // but we do not need to do anything here.
    MooseEigenSystem * esys =
        dynamic_cast<MooseEigenSystem *>(&_problem->getNonlinearSystemBase(/*nl_sys=*/0));
    if (esys)
      esys->markEigenVariable(var_name);
  }
}

std::set<SubdomainID>
AddVariableAction::getSubdomainIDs()
{
  // Extract and return the block ids supplied in the input
  std::set<SubdomainID> blocks;
  std::vector<SubdomainName> block_param =
      _moose_object_pars.get<std::vector<SubdomainName>>("block");
  for (const auto & subdomain_name : block_param)
  {
    SubdomainID blk_id = _problem->mesh().getSubdomainID(subdomain_name);
    blocks.insert(blk_id);
  }
  return blocks;
}
