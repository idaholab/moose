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
#include "MooseVariable.h"
#include "OversampleOutput.h"
#include "PetscOutput.h"
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

  // 'Variables' Group
  params.addParamNamesToGroup("hide show", "Variables");

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

template <>
InputParameters
validParams<AdvancedOutput<OversampleOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<OversampleOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template <>
InputParameters
validParams<AdvancedOutput<FileOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<FileOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template <>
InputParameters
validParams<AdvancedOutput<PetscOutput>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<PetscOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template <>
InputParameters
validParams<AdvancedOutput<Output>>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<Output>();
  addAdvancedOutputParams(params);
  return params;
}

// Defines the output types to enable for the AdvancedOutput object
template <class T>
MultiMooseEnum
AdvancedOutput<T>::getOutputTypes()
{
  return MultiMooseEnum("nodal=0 elemental=1 scalar=2 postprocessor=3 vector_postprocessor=4 "
                        "input=5 system_information=6");
}

// Enables the output types (see getOutputTypes) for an AdvancedOutput object
template <class T>
InputParameters
AdvancedOutput<T>::enableOutputTypes(const std::string & names)
{
  // The parameters object that will be returned
  InputParameters params = emptyInputParameters();

  // Set private parameter indicating that this method was called
  params.addPrivateParam("_execute_valid_params_was_called", true);

  // Get the MultiEnum of output types
  MultiMooseEnum output_types = AdvancedOutput<T>::getOutputTypes();

  // Update the enum of output types to append
  if (names.empty())
    output_types = output_types.getRawNames();
  else
    output_types = names;

  // Add the parameters and return them
  AdvancedOutput::addValidParams(params, output_types);
  return params;
}

// Constructor
template <class T>
AdvancedOutput<T>::AdvancedOutput(const InputParameters & parameters) : T(parameters)
{
  T::_is_advanced = true;
  T::_advanced_execute_on = OutputOnWarehouse(T::_execute_on, parameters);
}

template <class T>
void
AdvancedOutput<T>::initialSetup()
{
  // Do not initialize more than once
  // This check is needed for YAK which calls Executioners from within Executioners
  if (T::_initialized)
    return;

  // Check that enable[disable]OutputTypes was called
  if (!T::isParamValid("_execute_valid_params_was_called"))
    mooseError("The static method AdvancedOutput<T>::enableOutputTypes must be called inside the "
               "validParams function for this object to properly define the input parameters for "
               "the output object named '",
               T::name(),
               "'");

  // Initialize the available output
  initAvailableLists();

  // Separate the hide/show list into components
  initShowHideLists(T::template getParam<std::vector<VariableName>>("show"),
                    T::template getParam<std::vector<VariableName>>("hide"));

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (T::isParamValid("elemental_as_nodal") && T::template getParam<bool>("elemental_as_nodal"))
  {
    OutputData & nodal = _execute_data["nodal"];
    OutputData & elemental = _execute_data["elemental"];
    nodal.show.insert(elemental.show.begin(), elemental.show.end());
    nodal.hide.insert(elemental.hide.begin(), elemental.hide.end());
    nodal.available.insert(elemental.available.begin(), elemental.available.end());
  }

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable lists
  if (T::isParamValid("scalar_as_nodal") && T::template getParam<bool>("scalar_as_nodal"))
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
  for (auto & it : T::_advanced_execute_on)
    initExecutionTypes(it.first, it.second);

  // Set the initialization flag
  T::_initialized = true;
}

template <class T>
AdvancedOutput<T>::~AdvancedOutput()
{
}

template <class T>
void
AdvancedOutput<T>::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for the output object named '",
             T::name(),
             "'");
}

template <class T>
void
AdvancedOutput<T>::outputElementalVariables()
{
  mooseError(
      "Individual output of elemental variables is not support for this output object named '",
      T::name(),
      "'");
}

template <class T>
void
AdvancedOutput<T>::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for this output object named '",
             T::name(),
             "'");
}

template <class T>
void
AdvancedOutput<T>::outputVectorPostprocessors()
{
  mooseError(
      "Individual output of VectorPostprocessors is not support for this output object named '",
      T::name(),
      "'");
}

template <class T>
void
AdvancedOutput<T>::outputScalarVariables()
{
  mooseError(
      "Individual output of scalars is not support for this output object named '", T::name(), "'");
}

template <class T>
void
AdvancedOutput<T>::outputSystemInformation()
{
  mooseError(
      "Output of system information is not support for this output object named '", T::name(), "'");
}

template <class T>
void
AdvancedOutput<T>::outputInput()
{
  mooseError("Output of the input file information is not support for this output object named '",
             T::name(),
             "'");
}

// General outputStep() method
template <class T>
void
AdvancedOutput<T>::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!T::_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && T::_app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !T::onInterval())
    return;

  // Call output methods for various types
  output(type);
}

// FileOutput::outputStep specialization
template <>
void
AdvancedOutput<FileOutput>::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // Do nothing if the filename is not correct for output
  if (!FileOutput::checkFilename())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // Call output methods for various types
  output(type);
}

// OversampleOutput::outputStep specialization
template <>
void
AdvancedOutput<OversampleOutput>::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // Do nothing if the filename is not correct for output
  if (!checkFilename())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !onInterval())
    return;

  // Perform oversample solution projection
  updateOversample();

  // Call output methods for various types
  output(type);
}

template <class T>
void
AdvancedOutput<T>::output(const ExecFlagType & type)
{
  // Call the various output types, if data exists
  if (shouldOutput("nodal", type))
  {
    outputNodalVariables();
    _last_execute_time["nodal"] = T::_time;
  }

  if (shouldOutput("elemental", type))
  {
    outputElementalVariables();
    _last_execute_time["elemental"] = T::_time;
  }

  if (shouldOutput("postprocessors", type))
  {
    outputPostprocessors();
    _last_execute_time["postprocessors"] = T::_time;
  }

  if (shouldOutput("vector_postprocessors", type))
  {
    outputVectorPostprocessors();
    _last_execute_time["vector_postprocessors"] = T::_time;
  }

  if (shouldOutput("scalars", type))
  {
    outputScalarVariables();
    _last_execute_time["scalars"] = T::_time;
  }

  if (shouldOutput("system_information", type))
  {
    outputSystemInformation();
    _last_execute_time["system_information"] = T::_time;
  }

  if (shouldOutput("input", type))
  {
    outputInput();
    _last_execute_time["input"] = T::_time;
  }
}

template <class T>
bool
AdvancedOutput<T>::shouldOutput(const std::string & name, const ExecFlagType & type)
{
  // Ignore EXEC_FORCED for system information and input, there is no reason to force this
  if (type == EXEC_FORCED && (name == "system_information" || name == "input"))
    return false;

  // Do not output if the 'none' is contained by the execute_on
  if (T::_advanced_execute_on.contains(name) && T::_advanced_execute_on[name].contains("none"))
    return false;

  // Data output flag, true if data exists to be output
  bool execute_data_flag = true;

  // Set flag to false, if the OutputData exists and the output variable list is empty
  std::map<std::string, OutputData>::const_iterator iter = _execute_data.find(name);
  if (iter != _execute_data.end() && iter->second.output.empty())
    execute_data_flag = false;

  // Set flag to false, if the OutputOnWarehouse DOES NOT contain an entry
  if (!T::_advanced_execute_on.contains(name))
    execute_data_flag = false;

  // Force the output, if there is something to output and the time has not been output
  if (type == EXEC_FORCED && execute_data_flag && _last_execute_time[name] != T::_time)
    return true;

  // Return true (output should occur) if three criteria are satisfied, else do not output:
  //   (1) The execute_data_flag = true (i.e, there is data to output)
  //   (2) The current output type is contained in the list of output execution types
  //   (3) The current execution time is "final" or "forced" and the data has not already been
  //   output
  if (execute_data_flag && T::_advanced_execute_on[name].contains(type) &&
      !(type == EXEC_FINAL && _last_execute_time[name] == T::_time))
    return true;
  else
    return false;
}

template <class T>
bool
AdvancedOutput<T>::hasOutput(const ExecFlagType & type)
{
  // If any of the component outputs are true, then there is some output to perform
  for (const auto & it : T::_advanced_execute_on)
    if (shouldOutput(it.first, type))
      return true;
  // There is nothing to output
  return false;
}

template <class T>
bool
AdvancedOutput<T>::hasOutput()
{
  // Test that variables exist for output AND that output execution flags are valid
  for (const auto & it : _execute_data)
    if (!(it.second).output.empty() && T::_advanced_execute_on.contains(it.first) &&
        T::_advanced_execute_on[it.first].isValid())
      return true;

  // Test execution flags for non-variable output
  if (T::_advanced_execute_on.contains("system_information") &&
      T::_advanced_execute_on["system_information"].isValid())
    return true;
  if (T::_advanced_execute_on.contains("input") && T::_advanced_execute_on["input"].isValid())
    return true;

  return false;
}

template <class T>
void
AdvancedOutput<T>::initAvailableLists()
{
  // Initialize Postprocessor list
  // This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if postprocessor output is disabled
  initPostprocessorOrVectorPostprocessorLists<Postprocessor>("postprocessors");

  // Initialize vector postprocessor list
  // This flag is set to true if any vector postprocessor has the 'outputs' parameter set, it is
  // then used
  // to produce an warning if vector postprocessor output is disabled
  initPostprocessorOrVectorPostprocessorLists<VectorPostprocessor>("vector_postprocessors");

  // Get a list of the available variables
  std::vector<VariableName> variables = T::_problem_ptr->getVariableNames();

  // Loop through the variables and store the names in the correct available lists
  for (const auto & var_name : variables)
  {
    if (T::_problem_ptr->hasVariable(var_name))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, var_name);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _execute_data["elemental"].available.insert(var_name);
      else
        _execute_data["nodal"].available.insert(var_name);
    }

    else if (T::_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].available.insert(var_name);
  }
}

template <class T>
void
AdvancedOutput<T>::initExecutionTypes(const std::string & name, MultiMooseEnum & input)
{
  // Build the input paramemter name
  std::string param_name = "execute_";
  param_name += name + "_on";

  // The parameters exists and has been set by the user
  if (T::_pars.template have_parameter<MultiMooseEnum>(param_name) && T::isParamValid(param_name))
    input = T::template getParam<MultiMooseEnum>(param_name);

  // If the parameter does not exists; set it to a state where no valid entries exists so nothing
  // gets executed
  else if (!T::_pars.template have_parameter<MultiMooseEnum>(param_name))
  {
    input = T::_execute_on;
    input.clear();
  }
}

template <class T>
void
AdvancedOutput<T>::initShowHideLists(const std::vector<VariableName> & show,
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
    if (T::_problem_ptr->hasVariable(var_name))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, var_name);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _execute_data["elemental"].show.insert(var_name);
      else
        _execute_data["nodal"].show.insert(var_name);
    }
    else if (T::_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].show.insert(var_name);
    else if (T::_problem_ptr->hasPostprocessor(var_name))
      _execute_data["postprocessors"].show.insert(var_name);
    else if (T::_problem_ptr->hasVectorPostprocessor(var_name))
      _execute_data["vector_postprocessors"].show.insert(var_name);
    else
      unknown.insert(var_name);
  }

  // Populate the hide lists
  for (const auto & var_name : hide)
  {
    if (T::_problem_ptr->hasVariable(var_name))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, var_name);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _execute_data["elemental"].hide.insert(var_name);
      else
        _execute_data["nodal"].hide.insert(var_name);
    }
    else if (T::_problem_ptr->hasScalarVariable(var_name))
      _execute_data["scalars"].hide.insert(var_name);
    else if (T::_problem_ptr->hasPostprocessor(var_name))
      _execute_data["postprocessors"].hide.insert(var_name);
    else if (T::_problem_ptr->hasVectorPostprocessor(var_name))
      _execute_data["vector_postprocessors"].hide.insert(var_name);
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

template <class T>
void
AdvancedOutput<T>::initOutputList(OutputData & data)
{
  // References to the vectors of variable names
  std::set<std::string> & hide = data.hide;
  std::set<std::string> & show = data.show;
  std::set<std::string> & avail = data.available;
  std::set<std::string> & output = data.output;

  // Append the list from OutputInterface objects
  std::set<std::string> interface_hide;
  T::_app.getOutputWarehouse().buildInterfaceHideVariables(T::name(), interface_hide);
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

template <class T>
void
AdvancedOutput<T>::addValidParams(InputParameters & params, const MultiMooseEnum & types)
{
  MultiMooseEnum empty_execute_on(MooseUtils::createExecuteOnEnum({}, {EXEC_FINAL, EXEC_FAILED}));

  // Nodal output
  if (types.contains("nodal"))
  {
    params.addParam<MultiMooseEnum>(
        "execute_nodal_on", empty_execute_on, "Control the output of nodal variables");
    params.addParamNamesToGroup("execute_nodal_on", "Variables");
  }

  // Elemental output
  if (types.contains("elemental"))
  {
    params.addParam<MultiMooseEnum>(
        "execute_elemental_on", empty_execute_on, "Control the output of elemental variables");
    params.addParamNamesToGroup("execute_elemental_on", "Variables");

    // Add material output control, which are output via elemental variables
    params.addParam<bool>("output_material_properties",
                          false,
                          "Flag indicating if material properties should be output");
    params.addParam<std::vector<std::string>>(
        "show_material_properties",
        "List of materialproperties that should be written to the output");
    params.addParamNamesToGroup("output_material_properties show_material_properties", "Materials");
  }

  // Scalar variable output
  if (types.contains("scalar"))
  {
    params.addParam<MultiMooseEnum>(
        "execute_scalars_on", empty_execute_on, "Control the output of scalar variables");
    params.addParamNamesToGroup("execute_scalars_on", "Variables");
  }

  // Nodal and scalar output
  if (types.contains("nodal") && types.contains("scalar"))
  {
    params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");
    params.addParamNamesToGroup("scalar_as_nodal", "Variables");
  }

  // Elemental and nodal
  if (types.contains("elemental") && types.contains("nodal"))
  {
    params.addParam<bool>(
        "elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");
    params.addParamNamesToGroup("elemental_as_nodal", "Variables");
  }

  // Postprocessors
  if (types.contains("postprocessor"))
  {
    params.addParam<MultiMooseEnum>(
        "execute_postprocessors_on", empty_execute_on, "Control of when postprocessors are output");
    params.addParamNamesToGroup("execute_postprocessors_on", "Variables");
  }

  // Vector Postprocessors
  if (types.contains("vector_postprocessor"))
  {
    params.addParam<MultiMooseEnum>("execute_vector_postprocessors_on",
                                    empty_execute_on,
                                    "Enable/disable the output of VectorPostprocessors");
    params.addParamNamesToGroup("execute_vector_postprocessors_on", "Variables");
  }

  // Input file
  if (types.contains("input"))
  {
    params.addParam<MultiMooseEnum>(
        "execute_input_on", empty_execute_on, "Enable/disable the output of the input file");
    params.addParamNamesToGroup("execute_input_on", "Variables");
  }

  // System Information
  if (types.contains("system_information"))
  {
    params.addParam<MultiMooseEnum>("execute_system_information_on",
                                    empty_execute_on,
                                    "Control when the output of the simulation information occurs");
    params.addParamNamesToGroup("execute_system_information_on", "Variables");
  }
}

template <class T>
bool
AdvancedOutput<T>::hasOutputHelper(const std::string & name)
{
  if (!T::_initialized)
    mooseError("The output object must be initialized before it may be determined if ",
               name,
               " output is enabled.");

  return !_execute_data[name].output.empty() && T::_advanced_execute_on.contains(name) &&
         T::_advanced_execute_on[name].isValid() && !T::_advanced_execute_on[name].contains("none");
}

template <class T>
bool
AdvancedOutput<T>::hasNodalVariableOutput()
{
  return hasOutputHelper("nodal");
}

template <class T>
const std::set<std::string> &
AdvancedOutput<T>::getNodalVariableOutput()
{
  return _execute_data["nodal"].output;
}

template <class T>
bool
AdvancedOutput<T>::hasElementalVariableOutput()
{
  return hasOutputHelper("elemental");
}

template <class T>
const std::set<std::string> &
AdvancedOutput<T>::getElementalVariableOutput()
{
  return _execute_data["elemental"].output;
}

template <class T>
bool
AdvancedOutput<T>::hasScalarOutput()
{
  return hasOutputHelper("scalars");
}

template <class T>
const std::set<std::string> &
AdvancedOutput<T>::getScalarOutput()
{
  return _execute_data["scalars"].output;
}

template <class T>
bool
AdvancedOutput<T>::hasPostprocessorOutput()
{
  return hasOutputHelper("postprocessors");
}

template <class T>
const std::set<std::string> &
AdvancedOutput<T>::getPostprocessorOutput()
{
  return _execute_data["postprocessors"].output;
}

template <class T>
bool
AdvancedOutput<T>::hasVectorPostprocessorOutput()
{
  return hasOutputHelper("vector_postprocessors");
}

template <class T>
const std::set<std::string> &
AdvancedOutput<T>::getVectorPostprocessorOutput()
{
  return _execute_data["vector_postprocessors"].output;
}

template <class T>
const OutputOnWarehouse &
AdvancedOutput<T>::advancedExecuteOn() const
{
  return T::_advanced_execute_on;
}

// Instantiate the four possible template classes
template class AdvancedOutput<Output>;
template class AdvancedOutput<PetscOutput>;
template class AdvancedOutput<FileOutput>;
template class AdvancedOutput<OversampleOutput>;
