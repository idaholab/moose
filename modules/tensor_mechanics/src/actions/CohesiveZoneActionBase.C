//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneActionBase.h"
#include "CommonCohesiveZoneAction.h"
#include "ActionWarehouse.h"
#include "AddAuxVariableAction.h"
#include "MooseEnum.h"
#include "MooseApp.h"
#include "InputParameterWarehouse.h"

// map vector name shortcuts to tensor material property names
const std::map<std::string, std::string>
    CohesiveZoneActionBase::_real_vector_cartesian_component_table = {
        {"traction", "traction_global"},
        {"jump", "displacement_jump_global"},
        {"pk1_traction", "PK1traction"}};

// map aux variable name prefixes to CZM vector scalar options and list of permitted tensor name
// shortcuts
const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
    CohesiveZoneActionBase::_vector_direction_table = {
        {"normal", {"Normal", {"traction", "jump"}}},
        {"tangent", {"Tangent", {"traction", "jump"}}}};

const std::vector<char> CohesiveZoneActionBase::_component_table = {'x', 'y', 'z'};

InputParameters
CohesiveZoneActionBase::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action to create an instance of the cohesive zone model kernel for "
                             "each displacement component");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "The nonlinear displacement variables for the problem");
  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strainType, "Strain formulation");

  // Advanced
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Whether to use automatic differentiation to compute the Jacobian");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<AuxVariableName>>("save_in_master",
                                                "The displacement residuals on the  master side");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_master", "The displacement diagonal preconditioner terms on the  master side");
  params.addParam<std::vector<AuxVariableName>>("save_in_slave",
                                                "The displacement residuals on the  slave side");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_slave", "The displacement diagonal preconditioner terms on the  slave side");
  params.addParamNamesToGroup("save_in_master diag_save_in_master save_in_slave diag_save_in_slave",
                              "Advanced");
  params.addParam<bool>("verbose", false, "Display extra information.");

  // Output
  params.addParam<MultiMooseEnum>("generate_output",
                                  CohesiveZoneActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain");
  params.addParam<MultiMooseEnum>(
      "material_output_order",
      CohesiveZoneActionBase::materialOutputOrders(),
      "Specifies the order of the FE shape function to use for this variable.");
  params.addParam<MultiMooseEnum>(
      "material_output_family",
      CohesiveZoneActionBase::materialOutputFamilies(),
      "Specifies the family of FE shape functions to use for this variable.");
  params.addParamNamesToGroup("generate_output material_output_order material_output_family",
                              "Output");
  params.addParam<MultiMooseEnum>("additional_generate_output",
                                  CohesiveZoneActionBase::outputPropertiesType(),
                                  "Add scalar quantity output for stress and/or strain (will be "
                                  "appended to the list in `generate_output`)");
  params.addParam<MultiMooseEnum>(
      "additional_material_output_order",
      CohesiveZoneActionBase::materialOutputOrders(),
      "Specifies the order of the FE shape function to use for this variable.");

  params.addParam<MultiMooseEnum>(
      "additional_material_output_family",
      CohesiveZoneActionBase::materialOutputFamilies(),
      "Specifies the family of FE shape functions to use for this variable.");

  params.addParamNamesToGroup("additional_generate_output additional_material_output_order "
                              "additional_material_output_family",
                              "Output");
  return params;
}

CohesiveZoneActionBase::CohesiveZoneActionBase(const InputParameters & params) : Action(params)
{
  // FIXME: suggest to use action of action to add this to avoid changing the input parameters in
  // the warehouse.
  const auto & parameters = _app.getInputParameterWarehouse().getInputParameters();
  InputParameters & pars(*(parameters.find(uniqueActionName())->second.get()));

  // check if a container block with common parameters is found
  auto action = _awh.getActions<CommonCohesiveZoneAction>();
  if (action.size() == 1)
    pars.applyParameters(action[0]->parameters());

  // append additional_generate_output to generate_output
  if (isParamValid("additional_generate_output"))
  {
    MultiMooseEnum generate_output = getParam<MultiMooseEnum>("generate_output");
    MultiMooseEnum additional_generate_output =
        getParam<MultiMooseEnum>("additional_generate_output");

    MultiMooseEnum material_output_order = getParam<MultiMooseEnum>("material_output_order");
    MultiMooseEnum additional_material_output_order =
        getParam<MultiMooseEnum>("additional_material_output_order");

    MultiMooseEnum material_output_family = getParam<MultiMooseEnum>("material_output_family");
    MultiMooseEnum additional_material_output_family =
        getParam<MultiMooseEnum>("additional_material_output_family");

    for (auto & output : additional_generate_output)
      generate_output.push_back(output);
    for (auto & order : additional_material_output_order)
      material_output_order.push_back(order);
    for (auto & family : additional_material_output_family)
      material_output_family.push_back(family);

    pars.set<MultiMooseEnum>("generate_output") = generate_output;
    pars.set<MultiMooseEnum>("material_output_order") = material_output_order;
    pars.set<MultiMooseEnum>("material_output_family") = material_output_family;
  }
}

MultiMooseEnum
CohesiveZoneActionBase::outputPropertiesType()
{
  std::string options = "";
  for (auto & vc : _real_vector_cartesian_component_table)
    for (unsigned int a = 0; a < 3; ++a)
      options += (options == "" ? "" : " ") + vc.first + '_' + _component_table[a];

  for (auto & vi : _vector_direction_table)
    for (auto & t : vi.second.second)
      options += " " + vi.first + "_" + t;

  return MultiMooseEnum(options, "", true);
}

MultiMooseEnum
CohesiveZoneActionBase::materialOutputOrders()
{
  auto orders = AddAuxVariableAction::getAuxVariableOrders().getRawNames();

  return MultiMooseEnum(orders);
}

MultiMooseEnum
CohesiveZoneActionBase::materialOutputFamilies()
{
  return MultiMooseEnum("MONOMIAL");
}
