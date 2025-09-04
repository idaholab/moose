//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterExtraction.h"
#include "ParameterRegistry.h"
#include "ParseUtils.h"
#include "InputParameters.h"
#include "Moose.h"
#include "MooseUtils.h"

namespace Moose::ParameterExtraction
{
ExtractionInfo
extract(const hit::Node & root,
        const hit::Node * const command_line_root,
        const hit::Node * const section_node,
        InputParameters & params)
{
  mooseAssert(root.isRoot(), "Is not the root node");
  if (command_line_root)
    mooseAssert(command_line_root->isRoot(), "Is not the root node");
  if (section_node)
    mooseAssert(section_node->type() == hit::NodeType::Section, "Node type should be a section");

  ExtractionInfo info;

  const auto global_params_node = Moose::ParseUtils::queryGlobalParamsNode(root);

  // Try to extract each parameter
  for (const auto & [name, par_unique_ptr] : params)
  {
    if (params.shouldIgnore(name))
      continue;

    const hit::Node * param_node = nullptr;

    for (const auto & param_name : params.paramAliases(name))
    {
      // Check for parameters under the given section, if a section
      // node was provided
      if (section_node)
      {
        if (const auto section_param_node = section_node->find(param_name);
            section_param_node && section_param_node->type() == hit::NodeType::Field &&
            section_param_node->parent() == section_node)
          param_node = section_param_node;
      }
      // No node found within the given section, check [GlobalParams]
      if (!param_node && global_params_node)
      {
        if (const auto global_node = global_params_node->find(param_name);
            global_node && Moose::ParseUtils::isGlobal(*global_params_node, *global_node))
          param_node = global_node;
      }

      // Found it
      if (param_node)
      {
        const auto fullpath = param_node->fullpath();
        const bool global = global_params_node
                                ? Moose::ParseUtils::isGlobal(*global_params_node, *param_node)
                                : false;

        // Due to idaholab/moose#31461, we need to associate parameters that were
        // pulled from the command line hit parameters with the command line tree.
        // Otherwise, the filenames for these parameters will be incorrect (when
        // overridden from command line, the filename will be the input file
        // if the parameter is also in input)
        if (command_line_root)
          if (const auto command_line_node =
                  Moose::ParseUtils::queryCommandLineNode(*param_node, *command_line_root))
            param_node = command_line_node;

        // Associate the node that we found with the parameters so that
        // messages can be formed with the context of this parameter in input
        params.setHitNode(param_name, *param_node, {});
        // Mark the parameter as set
        params.set_attributes(param_name, false);
        // Keep track of this variable as extracted so that we can report
        // errors for unused variables
        info.extracted_variables.emplace_back(fullpath);

        // Check for deprecated parameters if the parameter is not a global param
        if (!global)
          if (const auto deprecated_message = params.queryDeprecatedParamMessage(param_name))
          {
            std::string key = "";
            if (const auto object_type_ptr = params.queryObjectType())
              key += *object_type_ptr + "_";
            key += param_name;
            info.deprecated_params.emplace(key, *deprecated_message);
          }

        // Private parameter, don't set
        if (params.isPrivate(param_name))
        {
          // Error if it's not global
          if (!global)
            info.errors.emplace_back("parameter '" + fullpath + "' is private and cannot be set",
                                     param_node);
          continue;
        }

        // Set the value, capturing errors
        const auto param_field = dynamic_cast<const hit::Field *>(param_node);
        mooseAssert(param_field, "Is not a field");
        bool set_param = false;
        try
        {
          ParameterRegistry::get().set(*par_unique_ptr, *param_field);
          set_param = true;
        }
        catch (hit::Error & e)
        {
          info.errors.emplace_back(e.message, param_node);
        }
        catch (std::exception & e)
        {
          info.errors.emplace_back(e.what(), param_node);
        }

        // Break if we failed here and don't perform extra checks
        if (!set_param)
          break;

        // Command line parameters have special isParamValid logic, so
        // we need to set that they've been set here if so
        if (params.isCommandLineParameter(param_name))
          params.commandLineParamSet(param_name, fullpath, param_node, {});

        // Special setup for vector<VariableName>
        if (auto cast_par = dynamic_cast<InputParameters::Parameter<std::vector<VariableName>> *>(
                par_unique_ptr.get()))
          if (const auto error = params.setupVariableNames(cast_par->set(), *param_node, {}))
            info.errors.emplace_back(*error, param_node);

        // Possibly perform a range check if this parameter has one
        if (params.isRangeChecked(param_node->path()))
          if (const auto error = params.parameterRangeCheck(
                  *par_unique_ptr, param_node->fullpath(), param_node->path(), true))
            info.errors.emplace_back(error->second, param_node);

        // Don't check the other alises since we've found it
        break;
      }
    }

    // // Special casing when the parameter was not found
    // if (!param_node)
    // {
    //   // In the case where we have OutFileName but it wasn't actually found in the input filename,
    //   // we will populate it with the actual parsed filename which is available here in the
    //   // parser.
    //   if (auto out_par_ptr =
    //           dynamic_cast<InputParameters::Parameter<OutFileBase> *>(par_unique_ptr.get()))
    //   {
    //     const auto input_file_name = getPrimaryFileName();
    //     mooseAssert(input_file_name.size(), "Input Filename is empty");
    //     const auto pos = input_file_name.find_last_of('.');
    //     mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");
    //     out_par_ptr->set() = input_file_name.substr(0, pos) + "_out";
    //     params.set_attributes(name, false);
    //   }
    // }
  }

  // See if there are any auto build vectors that need to be created
  for (const auto & [param_name, base_name_num_repeat_pair] : params.getAutoBuildVectors())
  {
    const auto & [base_name, num_repeat] = base_name_num_repeat_pair;
    // We'll autogenerate values iff the requested vector is not valid but both the base and
    // number are valid
    if (!params.isParamValid(param_name) && params.isParamValid(base_name) &&
        params.isParamValid(num_repeat))
    {
      const auto vec_size = params.get<unsigned int>(num_repeat);
      const std::string & name = params.get<std::string>(base_name);

      std::vector<VariableName> variable_names(vec_size);
      for (const auto i : index_range(variable_names))
      {
        std::ostringstream oss;
        oss << name << i;
        variable_names[i] = oss.str();
      }

      // Finally set the autogenerated vector into the InputParameters object
      params.set<std::vector<VariableName>>(param_name) = variable_names;
    }
  }

  return info;
}

ExtractionInfo
extract(const hit::Node & root,
        const hit::Node * command_line_root,
        const std::string & prefix,
        InputParameters & params)
{
  const auto node = root.find(prefix);
  const auto section_node = node && node->type() == hit::NodeType::Section ? node : nullptr;
  return extract(root, command_line_root, section_node, params);
}

ExtractionInfo
extract(const hit::Node & root, const std::string & prefix, InputParameters & params)
{
  return extract(root, nullptr, prefix, params);
}
}
