//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Standard includes
#include <math.h>

// MOOSE includes
#include "AdvancedOutput.h"
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "FileMesh.h"
#include "FileOutput.h"
#include "InfixIterator.h"
#include "MooseApp.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "VectorPostprocessor.h"

// A function, only available in this file, for adding the AdvancedOutput parameters. This is
// used to eliminate code duplication between the difference specializations of the validParams
// function.
namespace
{
void
addAdvancedOutputParams(InputParameters & params)
{
  // Hide/show variable output options
  params.addParam<std::vector<VariableName>>(
      "hide",
      "A list of the variables and postprocessors that should NOT be output to the Exodus "
      "file (may include Variables, ScalarVariables, and Postprocessor names).");

  params.addParam<std::vector<VariableName>>(
      "show",
      "A list of the variables and postprocessors that should be output to the Exodus file "
      "(may include Variables, ScalarVariables, and Postprocessor names).");

  // Enable output of PP/VPP to JSON
  params.addParam<bool>(
      "postprocessors_as_reporters", false, "Output Postprocessors values as Reporter values.");
  params.addParam<bool>("vectorpostprocessors_as_reporters",
                        false,
                        "Output VectorsPostprocessors vectors as Reporter values.");

  // Group for selecting the output
  params.addParamNamesToGroup("hide show", "Selection/restriction of output");

  // Group for converting outputs
  params.addParamNamesToGroup("postprocessors_as_reporters vectorpostprocessors_as_reporters",
                              "Conversions before output");

  // **** DEPRECATED PARAMS ****
  params.addDeprecatedParam<bool>("output_postprocessors",
                                  true,
                                  "Enable/disable the output of postprocessors",
                                  "'execute_postprocessors_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_vector_postprocessors",
                                  true,
                                  "Enable/disable the output of vector postprocessors",
                                  "'execute_vector_postprocessors_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_system_information",
                                  true,
                                  "Enable/disable the output of the simulation information",
                                  "'execute_system_information_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_elemental_variables",
                                  true,
                                  "Enable/disable the output of elemental variables",
                                  "'execute_elemental_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_nodal_variables",
                                  true,
                                  "Enable/disable the output of nodal variables",
                                  "'execute_nodal_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_scalar_variables",
                                  true,
                                  "Enable/disable the output of aux scalar variables",
                                  "'execute_scalars_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("execute_input",
                                  true,
                                  "Enable/disable the output of input file information",
                                  "'execute_input_on' has replaced this parameter");
}
}

InputParameters
AdvancedOutput::validParams()
{
  // Get the parameters from the parent object
  InputParameters params = FileOutput::validParams();
  addAdvancedOutputParams(params);
  return params;
}

// Defines the output types to enable for the AdvancedOutput object
MultiMooseEnum
AdvancedOutput::getOutputTypes()
{
  return MultiMooseEnum("nodal=0 elemental=1 scalar=2 postprocessor=3 vector_postprocessor=4 "
                        "input=5 system_information=6 reporter=7");
}

// Enables the output types (see getOutputTypes) for an AdvancedOutput object
InputParameters
AdvancedOutput::enableOutputTypes(const std::string & names)
{
  // The parameters object that will be returned
  InputParameters params = emptyInputParameters();

  // Get the MultiEnum of output types
  MultiMooseEnum output_types = getOutputTypes();

  // Update the enum of output types to append
  if (names.empty())
    output_types = output_types.getRawNames();
  else
    output_types = names;

  // Add the parameters and return them
  addValidParams(params, output_types);
  return params;
}

// Constructor
AdvancedOutput::AdvancedOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _elemental_as_nodal(isParamValid("elemental_as_nodal") ? getParam<bool>("elemental_as_nodal")
                                                           : false),
    _scalar_as_nodal(isParamValid("scalar_as_nodal") ? getParam<bool>("scalar_as_nodal") : false),
    _reporter_data(_problem_ptr->getReporterData()),
    _postprocessors_as_reporters(getParam<bool>("postprocessors_as_reporters")),
    _vectorpostprocessors_as_reporters(getParam<bool>("vectorpostprocessors_as_reporters"))
{
  _is_advanced = true;
  _advanced_execute_on = OutputOnWarehouse(_execute_on, parameters);
}

void
AdvancedOutput::initialSetup()
{
  init();
}

void
AdvancedOutput::init()
{
  // Clear existing execute information lists
  _execute_data.reset();

  // Initialize the available output
  initAvailableLists();

  // Separate the hide/show list into components
  initShowHideLists(getParam<std::vector<VariableName>>("show"),
                    getParam<std::vector<VariableName>>("hide"));

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (_elemental_as_nodal)
  {
    OutputData & nodal = _execute_data["nodal"];
    OutputData & elemental = _execute_data["elemental"];
    nodal.show.insert(elemental.show.begin(), elemental.show.end());
    nodal.hide.insert(elemental.hide.begin(), elemental.hide.end());
    nodal.available.insert(elemental.available.begin(), elemental.available.end());
  }

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable lists
  if (_scalar_as_nodal)
  {
    OutputData & nodal = _execute_data["nodal"];
    OutputData & scalar = _execute_data["scalars"];
    nodal.show.insert(scalar.show.begin(), scalar.show.end());
    nodal.hide.insert(scalar.hide.begin(), scalar.hide.end());
    nodal.available.insert(scalar.available.begin(), scalar.available.end());
  }

  // Initialize the show/hide/output lists for each of the types of output
  for (auto & it : _execute_data)
    initOutputList(it.second);

  // Initialize the execution flags
  for (auto & it : _advanced_execute_on)
    initExecutionTypes(it.first, it.second);
}

AdvancedOutput::~AdvancedOutput() {}

void
AdvancedOutput::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for the output object named '",
             name(),
             "'");
}

void
AdvancedOutput::outputElementalVariables()
{
  mooseError(
      "Individual output of elemental variables is not support for this output object named '",
      name(),
      "'");
}

void
AdvancedOutput::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for this output object named '",
             name(),
             "'");
}

void
AdvancedOutput::outputVectorPostprocessors()
{
  mooseError(
      "Individual output of VectorPostprocessors is not support for this output object named '",
      name(),
      "'");
}

void
AdvancedOutput::outputScalarVariables()
{
  mooseError(
      "Individual output of scalars is not support for this output object named '", name(), "'");
}

void
AdvancedOutput::outputSystemInformation()
{
  mooseError(
      "Output of system information is not support for this output object named '", name(), "'");
}

void
AdvancedOutput::outputInput()
{
  mooseError("Output of the input file information is not support for this output object named '",
             name(),
             "'");
}

void
AdvancedOutput::outputReporters()
{
  mooseError(
      "Output of the Reporter value(s) is not support for this output object named '", name(), "'");
}

bool
AdvancedOutput::shouldOutput(const ExecFlagType & type)
{
  if (!checkFilename())
    return false;

  if (hasOutput(type))
    return true;
  else
    return Output::shouldOutput(type);
}

void
AdvancedOutput::output(const ExecFlagType & type)
{
  // (re)initialize the list of available items for output
  init();

  // Call the various output types, if data exists
  if (wantOutput("nodal", type))
  {
    outputNodalVariables();
    _last_execute_time["nodal"] = _time;
  }

  if (wantOutput("elemental", type))
  {
    outputElementalVariables();
    _last_execute_time["elemental"] = _time;
  }

  if (wantOutput("postprocessors", type))
  {
    outputPostprocessors();
    _last_execute_time["postprocessors"] = _time;
  }

  if (wantOutput("vector_postprocessors", type))
  {
    outputVectorPostprocessors();
    _last_execute_time["vector_postprocessors"] = _time;
  }

  if (wantOutput("scalars", type))
  {
    outputScalarVariables();
    _last_execute_time["scalars"] = _time;
  }

  if (wantOutput("system_information", type))
  {
    outputSystemInformation();
    _last_execute_time["system_information"] = _time;
  }

  if (wantOutput("input", type))
  {
    outputInput();
    _last_execute_time["input"] = _time;
  }

  if (wantOutput("reporters", type))
  {
    outputReporters();
    _last_execute_time["reporters"] = _time;
  }
}

bool
AdvancedOutput::wantOutput(const std::string & name, const ExecFlagType & type)
{
  // Ignore EXEC_FORCED for system information and input, there is no reason to force this
  if (type == EXEC_FORCED && (name == "system_information" || name == "input"))
    return false;

  // Do not output if the 'none' is contained by the execute_on
  if (_advanced_execute_on.contains(name) && _advanced_execute_on[name].contains("none"))
    return false;

  // Data output flag, true if data exists to be output
  bool execute_data_flag = true;

  // Set flag to false, if the OutputData exists and the output variable list is empty
  std::map<std::string, OutputData>::const_iterator iter = _execute_data.find(name);
  if (iter != _execute_data.end() && iter->second.output.empty())
    execute_data_flag = false;

  // Set flag to false, if the OutputOnWarehouse DOES NOT contain an entry
  if (!_advanced_execute_on.contains(name))
    execute_data_flag = false;

  // Force the output, if there is something to output and the time has not been output
  if (type == EXEC_FORCED && execute_data_flag && _last_execute_time[name] != _time)
    return true;

  // Return true (output should occur) if three criteria are satisfied, else do not output:
  //   (1) The execute_data_flag = true (i.e, there is data to output)
  //   (2) The current output type is contained in the list of output execution types
  //   (3) The current execution time is "final" or "forced" and the data has not already been
  //   output
  if (execute_data_flag && _advanced_execute_on[name].contains(type) &&
      !(type == EXEC_FINAL && _last_execute_time[name] == _time))
    return true;
  else
    return false;
}

bool
AdvancedOutput::hasOutput(const ExecFlagType & type)
{
  // If any of the component outputs are true, then there is some output to perform
  for (const auto & it : _advanced_execute_on)
    if (wantOutput(it.first, type))
      return true;

  // There is nothing to output
  return false;
}

bool
AdvancedOutput::hasOutput()
{
  // Test that variables exist for output AND that output execution flags are valid
  for (const auto & it : _execute_data)
    if (!(it.second).output.empty() && _advanced_execute_on.contains(it.first) &&
        _advanced_execute_on[it.first].isValid())
      return true;

  // Test execution flags for non-variable output
  if (_advanced_execute_on.contains("system_information") &&
      _advanced_execute_on["system_information"].isValid())
    return true;
  if (_advanced_execute_on.contains("input") && _advanced_execute_on["input"].isValid())
    return true;

  return false;
}

void
AdvancedOutput::initAvailableLists()
{
  // Initialize Postprocessor list
  // This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if postprocessor output is disabled
  if (!_postprocessors_as_reporters)
    initPostprocessorOrVectorPostprocessorLists<Postprocessor>("postprocessors");

  // Initialize vector postprocessor list
  // This flag is set to true if any vector postprocessor has the 'outputs' parameter set, it is
  // then used
  // to produce an warning if vector postprocessor output is disabled
  if (!_vectorpostprocessors_as_reporters)
    initPostprocessorOrVectorPostprocessorLists<VectorPostprocessor>("vector_postprocessors");

  // Get a list of the available variables
  std::vector<VariableName> variables = _problem_ptr->getVariableNames();

  // Loop through the variables and store the names in the correct available lists
  for (const auto & var_name : variables)
  {
    if (_problem_ptr->hasVariable(var_name))
    {
      MooseVariableFEBase & var = _problem_ptr->getVariable(
          0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      const FEType type = var.feType();
      for (unsigned int i = 0; i < var.count(); ++i)
      {
        VariableName vname = var_name;
        if (var.isArray())
          vname = SubProblem::arrayVariableComponent(var_name, i);

        if (type.order == CONSTANT && type.family != MONOMIAL_VEC)
          _execute_data["elemental"].available.insert(vname);
        else if (type.family == NEDELEC_ONE || type.family == LAGRANGE_VEC ||
                 type.family == MONOMIAL_VEC)
        {
          const auto geom_type =
              ((type.family == MONOMIAL_VEC) && (type.order == CONSTANT)) ? "elemental" : "nodal";
          switch (_es_ptr->get_mesh().spatial_dimension())
          {
            case 0:
            case 1:
              _execute_data[geom_type].available.insert(vname);
              break;
            case 2:
              _execute_data[geom_type].available.insert(vname + "_x");
              _execute_data[geom_type].available.insert(vname + "_y");
              break;
            case 3:
              _execute_data[geom_type].available.insert(vname + "_x");
              _execute_data[geom_type].available.insert(vname + "_y");
              _execute_data[geom_type].available.insert(vname + "_z");
              break;
          }
        }
        else
          _execute_data["nodal"].available.insert(vname);
      }
    }

    else if (_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].available.insert(var_name);
  }

  // Initialize Reporter name list
  for (auto && r_name : _reporter_data.getReporterNames())
    if ((_postprocessors_as_reporters || !r_name.isPostprocessor()) &&
        (_vectorpostprocessors_as_reporters || !r_name.isVectorPostprocessor()))
      _execute_data["reporters"].available.insert(r_name);
}

void
AdvancedOutput::initExecutionTypes(const std::string & name, ExecFlagEnum & input)
{
  // Build the input paramemter name
  std::string param_name = "execute_";
  param_name += name + "_on";

  // The parameters exists and has been set by the user
  if (_pars.have_parameter<ExecFlagEnum>(param_name) && isParamValid(param_name))
    input = getParam<ExecFlagEnum>(param_name);

  // If the parameter does not exists; set it to a state where no valid entries exists so nothing
  // gets executed
  else if (!_pars.have_parameter<ExecFlagEnum>(param_name))
  {
    input = _execute_on;
    input.clear();
  }
}

void
AdvancedOutput::initShowHideLists(const std::vector<VariableName> & show,
                                  const std::vector<VariableName> & hide)
{

  // Storage for user-supplied input that is unknown as a variable or postprocessor
  std::set<std::string> unknown;

  // If a show hide/list exists, let the data warehouse know about it. This allows for the proper
  // handling of output lists (see initOutputList)
  if (show.size() > 0)
    _execute_data.setHasShowList(true);

  // Populate the show lists
  for (const auto & var_name : show)
  {
    if (_problem_ptr->hasVariable(var_name))
    {
      MooseVariableFEBase & var = _problem_ptr->getVariable(
          0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      const FEType type = var.feType();
      for (unsigned int i = 0; i < var.count(); ++i)
      {
        VariableName vname = var_name;
        if (var.isArray())
          vname = SubProblem::arrayVariableComponent(var_name, i);

        if (type.order == CONSTANT)
          _execute_data["elemental"].show.insert(vname);
        else if (type.family == NEDELEC_ONE || type.family == LAGRANGE_VEC ||
                 type.family == MONOMIAL_VEC)
        {
          const auto geom_type =
              ((type.family == MONOMIAL_VEC) && (type.order == CONSTANT)) ? "elemental" : "nodal";
          switch (_es_ptr->get_mesh().spatial_dimension())
          {
            case 0:
            case 1:
              _execute_data[geom_type].show.insert(vname);
              break;
            case 2:
              _execute_data[geom_type].show.insert(vname + "_x");
              _execute_data[geom_type].show.insert(vname + "_y");
              break;
            case 3:
              _execute_data[geom_type].show.insert(vname + "_x");
              _execute_data[geom_type].show.insert(vname + "_y");
              _execute_data[geom_type].show.insert(vname + "_z");
              break;
          }
        }
        else
          _execute_data["nodal"].show.insert(vname);
      }
    }
    else if (_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].show.insert(var_name);
    else if (hasPostprocessorByName(var_name))
      _execute_data["postprocessors"].show.insert(var_name);
    else if (hasVectorPostprocessorByName(var_name))
      _execute_data["vector_postprocessors"].show.insert(var_name);
    else if ((var_name.find("/") != std::string::npos) &&
             (hasReporterValueByName(ReporterName(var_name))))
      _execute_data["reporters"].show.insert(var_name);
    else
      unknown.insert(var_name);
  }

  // Populate the hide lists
  for (const auto & var_name : hide)
  {
    if (_problem_ptr->hasVariable(var_name))
    {
      MooseVariableFEBase & var = _problem_ptr->getVariable(
          0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      const FEType type = var.feType();
      for (unsigned int i = 0; i < var.count(); ++i)
      {
        VariableName vname = var_name;
        if (var.isArray())
          vname = SubProblem::arrayVariableComponent(var_name, i);

        if (type.order == CONSTANT)
          _execute_data["elemental"].hide.insert(vname);
        else if (type.family == NEDELEC_ONE || type.family == LAGRANGE_VEC ||
                 type.family == MONOMIAL_VEC)
        {
          switch (_es_ptr->get_mesh().spatial_dimension())
          {
            case 0:
            case 1:
              _execute_data["nodal"].hide.insert(vname);
              break;
            case 2:
              _execute_data["nodal"].hide.insert(vname + "_x");
              _execute_data["nodal"].hide.insert(vname + "_y");
              break;
            case 3:
              _execute_data["nodal"].hide.insert(vname + "_x");
              _execute_data["nodal"].hide.insert(vname + "_y");
              _execute_data["nodal"].hide.insert(vname + "_z");
              break;
          }
        }
        else
          _execute_data["nodal"].hide.insert(vname);
      }
    }
    else if (_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].hide.insert(var_name);
    else if (hasPostprocessorByName(var_name))
      _execute_data["postprocessors"].hide.insert(var_name);
    else if (hasVectorPostprocessorByName(var_name))
      _execute_data["vector_postprocessors"].hide.insert(var_name);
    else if ((var_name.find("/") != std::string::npos) &&
             (hasReporterValueByName(ReporterName(var_name))))
      _execute_data["reporters"].hide.insert(var_name);

    else
      unknown.insert(var_name);
  }

  // Error if an unknown variable or postprocessor is found
  if (!unknown.empty())
  {
    std::ostringstream oss;
    oss << "Output(s) do not exist (must be variable, scalar, postprocessor, or vector "
           "postprocessor): ";
    std::copy(unknown.begin(), unknown.end(), infix_ostream_iterator<std::string>(oss, " "));
    mooseError(oss.str());
  }
}

void
AdvancedOutput::initOutputList(OutputData & data)
{
  // References to the vectors of variable names
  std::set<std::string> & hide = data.hide;
  std::set<std::string> & show = data.show;
  std::set<std::string> & avail = data.available;
  std::set<std::string> & output = data.output;

  // Append to the hide list from OutputInterface objects
  std::set<std::string> interface_hide;
  _app.getOutputWarehouse().buildInterfaceHideVariables(name(), interface_hide);
  hide.insert(interface_hide.begin(), interface_hide.end());

  // Both show and hide are empty and no show/hide settings were provided (show all available)
  if (show.empty() && hide.empty() && !_execute_data.hasShowList())
    output = avail;

  // Only hide is empty (show all the variables listed)
  else if (!show.empty() && hide.empty())
    output = show;

  // Only show is empty (show all except those hidden)
  else if (show.empty() && !hide.empty())
    std::set_difference(avail.begin(),
                        avail.end(),
                        hide.begin(),
                        hide.end(),
                        std::inserter(output, output.begin()));

  // Both hide and show are present (show all those listed)
  else
  {
    // Check if variables are in both, which is invalid
    std::vector<std::string> tmp;
    std::set_intersection(
        hide.begin(), hide.end(), show.begin(), show.end(), std::inserter(tmp, tmp.begin()));
    if (!tmp.empty())
    {
      std::ostringstream oss;
      oss << "Output(s) specified to be both shown and hidden: ";
      std::copy(tmp.begin(), tmp.end(), infix_ostream_iterator<std::string>(oss, " "));
      mooseError(oss.str());
    }

    // Define the output variable list
    output = show;
  }
}

void
AdvancedOutput::addValidParams(InputParameters & params, const MultiMooseEnum & types)
{
  ExecFlagEnum empty_execute_on = MooseUtils::getDefaultExecFlagEnum();
  empty_execute_on.addAvailableFlags(EXEC_FAILED);

  // Nodal output
  if (types.contains("nodal"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_nodal_on", empty_execute_on, "Control the output of nodal variables");
    params.addParamNamesToGroup("execute_nodal_on", "Selection/restriction of output");
  }

  // Elemental output
  if (types.contains("elemental"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_elemental_on", empty_execute_on, "Control the output of elemental variables");
    params.addParamNamesToGroup("execute_elemental_on", "Selection/restriction of output");

    // Add material output control, which are output via elemental variables
    params.addParam<bool>("output_material_properties",
                          false,
                          "Flag indicating if material properties should be output");
    params.addParam<std::vector<std::string>>(
        "show_material_properties",
        "List of material properties that should be written to the output");
    params.addParamNamesToGroup("output_material_properties show_material_properties", "Materials");

    // Add mesh extra element id control, which are output via elemental variables
    params.addParam<bool>(
        "output_extra_element_ids",
        false,
        "Flag indicating if extra element ids defined on the mesh should be outputted");
    params.addParam<std::vector<std::string>>(
        "extra_element_ids_to_output",
        "List of extra element ids defined on the mesh that should be written to the output.");
    params.addParamNamesToGroup("output_extra_element_ids extra_element_ids_to_output", "Mesh");
  }

  // Scalar variable output
  if (types.contains("scalar"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_scalars_on", empty_execute_on, "Control the output of scalar variables");
    params.addParamNamesToGroup("execute_scalars_on", "Selection/restriction of output");
  }

  // Nodal and scalar output
  if (types.contains("nodal") && types.contains("scalar"))
  {
    params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");
    params.addParamNamesToGroup("scalar_as_nodal", "Conversions before output");
  }

  // Elemental and nodal
  if (types.contains("elemental") && types.contains("nodal"))
  {
    params.addParam<bool>(
        "elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");
    params.addParamNamesToGroup("elemental_as_nodal", "Conversions before output");
  }

  // Postprocessors
  if (types.contains("postprocessor"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_postprocessors_on", empty_execute_on, "Control of when postprocessors are output");
    params.addParamNamesToGroup("execute_postprocessors_on", "Selection/restriction of output");
  }

  // Vector Postprocessors
  if (types.contains("vector_postprocessor"))
  {
    params.addParam<ExecFlagEnum>("execute_vector_postprocessors_on",
                                  empty_execute_on,
                                  "Enable/disable the output of VectorPostprocessors");
    params.addParamNamesToGroup("execute_vector_postprocessors_on",
                                "Selection/restriction of output");
  }

  // Reporters
  if (types.contains("reporter"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_reporters_on", empty_execute_on, "Control of when Reporter values are output");
    params.addParamNamesToGroup("execute_reporters_on", "Selection/restriction of output");
  }

  // Input file
  if (types.contains("input"))
  {
    params.addParam<ExecFlagEnum>(
        "execute_input_on", empty_execute_on, "Enable/disable the output of the input file");
    params.addParamNamesToGroup("execute_input_on", "Selection/restriction of output");
  }

  // System Information
  if (types.contains("system_information"))
  {
    params.addParam<ExecFlagEnum>("execute_system_information_on",
                                  empty_execute_on,
                                  "Control when the output of the simulation information occurs");
    params.addParamNamesToGroup("execute_system_information_on", "Selection/restriction of output");
  }
}

bool
AdvancedOutput::hasOutputHelper(const std::string & name)
{
  return !_execute_data[name].output.empty() && _advanced_execute_on.contains(name) &&
         _advanced_execute_on[name].isValid() && !_advanced_execute_on[name].contains("none");
}

bool
AdvancedOutput::hasNodalVariableOutput()
{
  return hasOutputHelper("nodal");
}

const std::set<std::string> &
AdvancedOutput::getNodalVariableOutput()
{
  return _execute_data["nodal"].output;
}

bool
AdvancedOutput::hasElementalVariableOutput()
{
  return hasOutputHelper("elemental");
}

const std::set<std::string> &
AdvancedOutput::getElementalVariableOutput()
{
  return _execute_data["elemental"].output;
}

bool
AdvancedOutput::hasScalarOutput()
{
  return hasOutputHelper("scalars");
}

const std::set<std::string> &
AdvancedOutput::getScalarOutput()
{
  return _execute_data["scalars"].output;
}

bool
AdvancedOutput::hasPostprocessorOutput()
{
  return hasOutputHelper("postprocessors");
}

const std::set<std::string> &
AdvancedOutput::getPostprocessorOutput()
{
  return _execute_data["postprocessors"].output;
}

bool
AdvancedOutput::hasVectorPostprocessorOutput()
{
  return hasOutputHelper("vector_postprocessors");
}

const std::set<std::string> &
AdvancedOutput::getVectorPostprocessorOutput()
{
  return _execute_data["vector_postprocessors"].output;
}

bool
AdvancedOutput::hasReporterOutput()
{
  return hasOutputHelper("reporters");
}

const std::set<std::string> &
AdvancedOutput::getReporterOutput()
{
  return _execute_data["reporters"].output;
}

const OutputOnWarehouse &
AdvancedOutput::advancedExecuteOn() const
{
  return _advanced_execute_on;
}
