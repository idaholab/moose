//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialOutputAction.h"
#include "FEProblem.h"
#include "FEProblemBase.h"
#include "MooseApp.h"
#include "AddOutputAction.h"
#include "MaterialBase.h"
#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankFourTensor.h"
#include "MooseEnum.h"
#include "MooseVariableConstMonomial.h"
#include "FunctorMaterial.h"

#include "libmesh/utility.h"

/// List of AuxKernels used for the respective property type output (one entry for each type in SupportedTypes)
const std::vector<std::string> MaterialOutputAction::_aux_kernel_names = {
    "MaterialRealAux",
    "MaterialRealVectorValueAux",
    "MaterialRealTensorValueAux",
    "MaterialRankTwoTensorAux",
    "MaterialRankFourTensorAux",
    "MaterialSymmetricRankTwoTensorAux",
    "MaterialSymmetricRankFourTensorAux"};

/// List of index symbols (one entry for each type in SupportedTypes)
const std::vector<std::string> MaterialOutputAction::_index_symbols = {
    "", "xyz", "012", "012", "012", "012345", "012345"};

/// List of coefficient parameter names (one entry for each type in SupportedTypes)
const std::vector<std::vector<std::string>> MaterialOutputAction::_param_names = {
    {},
    {"component"},
    {"row", "column"},
    {"i", "j"},
    {"i", "j", "k", "l"},
    {"component"},
    {"i", "j"}};

registerMooseAction("MooseApp", MaterialOutputAction, "add_output_aux_variables");
registerMooseAction("MooseApp", MaterialOutputAction, "add_aux_kernel");

InputParameters
MaterialOutputAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

MaterialOutputAction::MaterialOutputAction(const InputParameters & params)
  : Action(params),
    _block_material_data(nullptr),
    _boundary_material_data(nullptr),
    _output_warehouse(_app.getOutputWarehouse()),
    _output_only_on_timestep_end(_app.parameters().get<bool>("use_legacy_material_output"))
{
}

void
MaterialOutputAction::act()
{
  mooseAssert(_problem,
              "FEProblemBase pointer is nullptr, it is needed for auto material property output");

  // Do nothing if the application does not have output
  if (!_app.actionWarehouse().hasActions("add_output"))
    return;

  // Set the pointers to the MaterialData objects (Note, these pointers are not available at
  // construction)
  _block_material_data = &_problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  _boundary_material_data = &_problem->getMaterialData(Moose::BOUNDARY_MATERIAL_DATA);

  // A complete list of all MaterialBase objects
  const auto & material_ptrs = _problem->getMaterialWarehouse().getObjects();

  // Handle setting of material property output in [Outputs] sub-blocks
  // Output objects can enable material property output, the following code examines the parameters
  // for each Output object and sets a flag if any Output object has output set and also builds a
  // list if the
  // properties are limited via the 'show_material_properties' parameters
  bool outputs_has_properties = false;
  std::set<std::string> output_object_properties;

  const auto & output_actions = _app.actionWarehouse().getActionListByName("add_output");
  for (const auto & act : output_actions)
  {
    // Extract the Output action
    AddOutputAction * action = dynamic_cast<AddOutputAction *>(act);
    if (!action)
      continue;

    // Add the material property names from the output object parameters to the list of properties
    // to output
    InputParameters & params = action->getObjectParams();
    if (params.isParamValid("output_material_properties") &&
        params.get<bool>("output_material_properties"))
    {
      outputs_has_properties = true;
      std::vector<std::string> prop_names =
          params.get<std::vector<std::string>>("show_material_properties");
      output_object_properties.insert(prop_names.begin(), prop_names.end());
    }
  }

  // Loop through each material object
  std::set<std::string> unsupported_properties;
  std::set<std::string> material_property_names;
  for (const auto & mat : material_ptrs)
  {
    // Extract the names of the output objects to which the material properties will be exported
    std::set<OutputName> outputs = mat->getOutputs();

    // Extract the property names that will actually be output
    std::vector<std::string> output_properties =
        mat->getParam<std::vector<std::string>>("output_properties");

    // Append the properties listed in the Outputs block
    if (outputs_has_properties)
      output_properties.insert(output_properties.end(),
                               output_object_properties.begin(),
                               output_object_properties.end());

    // Clear the list of variable names for the current material object, this list will be populated
    // with all the variables names for the current material object and is needed for purposes of
    // controlling the which output objects show the material property data
    _material_variable_names.clear();

    // Create necessary outputs for the properties if:
    //   (1) The Outputs block has material output enabled
    //   (2) If the MaterialBase object itself has set the 'outputs' parameter
    if (outputs_has_properties || outputs.find("none") == outputs.end())
    {
      // Get all material properties supplied by this material as a starting point
      std::set<std::string> names = mat->getSuppliedItems();
      if (const auto fmat_ptr = dynamic_cast<const FunctorMaterial *>(mat.get()))
        names.insert(fmat_ptr->getSuppliedFunctors().begin(),
                     fmat_ptr->getSuppliedFunctors().end());
      for (const auto & name : names)
      {
        // Output the property only if the name is contained in the 'output_properties'
        // list or if the list is empty (all properties)
        if (output_properties.empty() ||
            std::find(output_properties.begin(), output_properties.end(), name) !=
                output_properties.end())
        { // Add the material property for output
          Moose::typeLoop<SetupOutput>(SupportedTypes{}, this, name, std::cref(*mat));
          material_property_names.insert(name);
        }
      }
      // If the material object has limited outputs, store the variables associated with the
      // output objects
      if (!outputs.empty())
        for (const auto & output_name : outputs)
          _material_variable_names_map[output_name].insert(_material_variable_names.begin(),
                                                           _material_variable_names.end());
    }

    for (const auto & name : _material_variable_names)
      _all_variable_names.insert(name);

    for (const auto & name : material_property_names)
      if (_supported_properties.count(name) == 0)
        unsupported_properties.insert(name);
  }

  if (_current_task == "add_output_aux_variables")
  {
    // Check and warn for unsupported outputs
    if (unsupported_properties.size() > 0)
    {
      std::string names;
      for (const auto & name : unsupported_properties)
        names += "\n  " + name;
      mooseWarning("The types for total ",
                   unsupported_properties.size(),
                   " material properties:",
                   names,
                   "\nare not supported for automatic output by ",
                   type(),
                   ".");
    }

    if (_all_variable_names.size() > 0)
    {
      std::string names;
      for (const auto & name : _all_variable_names)
        names += "\n  " + name;
      _console << COLOR_CYAN << "The following total " << _all_variable_names.size()
               << " aux variables:" << names << "\nare added for automatic output by " << type()
               << "." << COLOR_DEFAULT << std::endl;
    }

    // When a MaterialBase object has 'output_properties' defined all other properties not listed
    // must be added to the hide list for the output objects so that properties that are not desired
    // do not appear.
    for (const auto & it : _material_variable_names_map)
    {
      std::set<std::string> hide;
      std::set_difference(_all_variable_names.begin(),
                          _all_variable_names.end(),
                          it.second.begin(),
                          it.second.end(),
                          std::inserter(hide, hide.begin()));

      _output_warehouse.addInterfaceHideVariables(it.first, hide);
    }
  }
}

template <typename T, int I>
void
MaterialOutputAction::setupOutput(const MaterialPropertyName & prop_name,
                                  const MaterialBase & material)
{
  if (hasProperty<T>(prop_name))
    setupOutputHelper<T, I, false, false>(prop_name, material);
  else if (hasADProperty<T>(prop_name))
    setupOutputHelper<T, I, true, false>(prop_name, material);
  else if (hasFunctorProperty<T>(prop_name))
    setupOutputHelper<T, I, false, true>(prop_name, material);
  else if (hasFunctorProperty<typename Moose::ADType<T>::type>(prop_name))
    setupOutputHelper<T, I, true, true>(prop_name, material);
}

namespace
{
template <typename T>
struct Size
{
  static constexpr std::size_t value = T::N;
};

template <>
struct Size<Real>
{
  static constexpr std::size_t value = 1;
};
template <>
struct Size<RealVectorValue>
{
  static constexpr std::size_t value = 3;
};
template <>
struct Size<RealTensorValue>
{
  static constexpr std::size_t value = 3;
};
}

template <typename T, int I, bool is_ad, bool is_functor>
void
MaterialOutputAction::setupOutputHelper(const MaterialPropertyName & prop_name,
                                        const MaterialBase & material)
{
  const auto dim = _param_names[I].size();
  std::array<std::size_t, 4> i;
  for (i[3] = 0; i[3] < (dim < 4 ? 1 : Size<T>::value); ++i[3])
    for (i[2] = 0; i[2] < (dim < 3 ? 1 : Size<T>::value); ++i[2])
      for (i[1] = 0; i[1] < (dim < 2 ? 1 : Size<T>::value); ++i[1])
        for (i[0] = 0; i[0] < (dim < 1 ? 1 : Size<T>::value); ++i[0])
        {
          std::string var_name = prop_name + (is_functor ? "_out" : "") + (dim ? "_" : "");
          for (const auto j : make_range(dim))
            var_name += Moose::stringify(_index_symbols[I][i[j]]);

          //  Add AuxVariables
          if (_current_task == "add_output_aux_variables")
          {
            if (_problem->hasVariable(var_name) && _all_variable_names.count(var_name) == 0)
              mooseError(
                  "The material property output " + var_name +
                  " has the same name as an existing variable, either use the material"
                  " declare_suffix parameter to disambiguate or the output_properties parameter"
                  " to restrict the material properties to output");

            auto params = _factory.getValidParams("MooseVariableConstMonomial");
            // currently only elemental variables are supported for material property output
            params.set<MooseEnum>("order") = "CONSTANT";
            params.set<MooseEnum>("family") = "MONOMIAL";
            _problem->addAuxVariable("MooseVariableConstMonomial", var_name, params);
          }

          // Add AuxKernel
          const auto kernel_name = (is_ad ? "AD" : "") + _aux_kernel_names[I];
          auto params = getParams<is_functor>(kernel_name, prop_name, var_name, material);
          if (_current_task == "add_aux_kernel")
          {
            for (const auto j : make_range(dim))
              params.template set<unsigned int>(_param_names[I][j]) = i[j];

            _problem->addAuxKernel(kernel_name, material.name() + var_name, params);
          }
        }

  _supported_properties.insert(prop_name);
}

template <bool is_functor>
InputParameters
MaterialOutputAction::getParams(const std::string & type,
                                const std::string & property_name,
                                const std::string & variable_name,
                                const MaterialBase & material)
{
  // Append the list of output variables for the current material
  _material_variable_names.insert(variable_name);

  // Set the action parameters
  InputParameters params = _factory.getValidParams(type);

  // Adapt for regular or functor materials
  if constexpr (is_functor)
    params.set<MooseFunctorName>("functor") = property_name;
  else
    params.set<MaterialPropertyName>("property") = property_name;

  params.set<AuxVariableName>("variable") = variable_name;
  if (_output_only_on_timestep_end)
    params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  else
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  if (material.boundaryRestricted())
    params.set<std::vector<BoundaryName>>("boundary") = material.boundaryNames();
  else
    params.set<std::vector<SubdomainName>>("block") = material.blocks();

  return params;
}
