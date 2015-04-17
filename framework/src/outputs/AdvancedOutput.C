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
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"
#include "CoupledExecutioner.h"
#include "VectorPostprocessor.h"
#include "MooseUtils.h"
#include "InfixIterator.h"

// A function, only available in this file, for adding the AdvancedOutput parameters. This is
// used to eliminate code duplication between the difference specializations of the validParams function.
namespace
{
void addAdvancedOutputParams(InputParameters & params)
{
  // Hide/show variable output options
  params.addParam<std::vector<VariableName> >("hide", "A list of the variables and postprocessors that should NOT be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

  params.addParam<std::vector<VariableName> >("show", "A list of the variables and postprocessors that should be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

  // 'Variables' Group
  params.addParamNamesToGroup("hide show", "Variables");

   // **** DEPRECATED PARAMS ****
  params.addDeprecatedParam<bool>("output_postprocessors", true, "Enable/disable the output of postprocessors",
                                  "'output_postprocessors_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_vector_postprocessors", true, "Enable/disable the output of vector postprocessors",
                                  "'output_vector_postprocessors_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_system_information", true, "Enable/disable the output of the simulation information",
                                  "'output_system_information_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_elemental_variables", true, "Enable/disable the output of elemental variables",
                                  "'output_elemental_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_nodal_variables", true, "Enable/disable the output of nodal variables",
                                  "'output_nodal_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_scalar_variables", true, "Enable/disable the output of aux scalar variables",
                                  "'output_scalars_on' has replaced this parameter");
  params.addDeprecatedParam<bool>("output_input", true, "Enable/disable the output of input file information",
                                  "'output_input_on' has replaced this parameter");
}
}

template<>
InputParameters validParams<AdvancedOutput<OversampleOutput> >()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<OversampleOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template<>
InputParameters validParams<AdvancedOutput<FileOutput> >()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<FileOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template<>
InputParameters validParams<AdvancedOutput<PetscOutput> >()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<PetscOutput>();
  addAdvancedOutputParams(params);
  return params;
}

template<>
InputParameters validParams<AdvancedOutput<Output> >()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<Output>();
  addAdvancedOutputParams(params);
  return params;
}

// Defines the output types to enable for the AdvancedOutput object
template<class T>
MultiMooseEnum
AdvancedOutput<T>::getOutputTypes()
{
  return MultiMooseEnum("nodal=0 elemental=1 scalar=2 postprocessor=3 vector_postprocessor=4 input=5 system_information=6");
}

// Enables the output types (see getOutputTypes) for an AdvancedOutput object
template<class T>
InputParameters
AdvancedOutput<T>::enableOutputTypes(const std::string & names)
{
  // The parameters object that will be returned
  InputParameters params = emptyInputParameters();

  // Set private parameter indicating that this method was called
  params.addPrivateParam("_output_valid_params_was_called", true);

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
template<class T>
AdvancedOutput<T>::AdvancedOutput(const std::string & name, InputParameters & parameters) :
    T(name, parameters)
{
  T::_is_advanced = true;
  T::_advanced_output_on = OutputOnWarehouse(T::_output_on, parameters);
}

template<class T>
void
AdvancedOutput<T>::initialSetup()
{
  // Do not initialize more than once
  // This check is needed for YAK which calls Executioners from within Executioners
  if (T::_initialized)
    return;

  // Check that enable[disable]OutputTypes was called
  if (!T::isParamValid("_output_valid_params_was_called"))
    mooseError("The static method AdvancedOutput<T>::enableOutputTypes must be called inside the validParams function for this object to properly define the input parameters for the output object named '" << T::_name << "'");

  // Initialize the available output
  initAvailableLists();

  // Separate the hide/show list into components
  initShowHideLists(T::template getParam<std::vector<VariableName> >("show"),
                    T::template getParam<std::vector<VariableName> >("hide"));

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (T::isParamValid("elemental_as_nodal") && T::template getParam<bool>("elemental_as_nodal"))
  {
    OutputData & nodal = _output_data["nodal"];
    OutputData & elemental = _output_data["elemental"];
    nodal.show.insert(elemental.show.begin(), elemental.show.end());
    nodal.hide.insert(elemental.hide.begin(), elemental.hide.end());
    nodal.available.insert(elemental.available.begin(), elemental.available.end());
  }

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable lists
  if (T::isParamValid("scalar_as_nodal") && T::template getParam<bool>("scalar_as_nodal"))
  {
    OutputData & nodal = _output_data["nodal"];
    OutputData & scalar = _output_data["scalars"];
    nodal.show.insert(scalar.show.begin(), scalar.show.end());
    nodal.hide.insert(scalar.hide.begin(), scalar.hide.end());
    nodal.available.insert(scalar.available.begin(), scalar.available.end());
  }

  // Initialize the show/hide/output lists for each of the types of output
  for (std::map<std::string, OutputData>::iterator it = _output_data.begin(); it != _output_data.end(); ++it)
    initOutputList(it->second);

  // Initialize the execution flags
  for (std::map<std::string, MultiMooseEnum>::iterator it = T::_advanced_output_on.begin(); it != T::_advanced_output_on.end(); ++it)
    initExecutionTypes(it->first, it->second);

  // Set the initialization flag
  T::_initialized = true;

  // **** DEPRECATED PARAMETER SUPPORT ****
  if (T::isParamValid("output_postprocessors") && !T::template getParam<bool>("output_postprocessors"))
    T::_advanced_output_on["postprocessors"].clear();
  if (T::isParamValid("output_vector_postprocessors") && !T::template getParam<bool>("output_vector_postprocessors"))
    T::_advanced_output_on["vector_postprocessors"].clear();
  if (T::isParamValid("output_scalar_variables") && !T::template getParam<bool>("output_scalar_variables"))
    T::_advanced_output_on["scalars"].clear();
  if (T::isParamValid("output_elemental_variables") && !T::template getParam<bool>("output_elemental_variables"))
    T::_advanced_output_on["elemental"].clear();
  if (T::isParamValid("output_nodal_variables") && !T::template getParam<bool>("output_nodal_variables"))
    T::_advanced_output_on["nodal"].clear();
  if (T::isParamValid("output_system_information") && !T::template getParam<bool>("output_system_information"))
    T::_advanced_output_on["system_information"].clear();
  if (T::isParamValid("output_input") && !T::template getParam<bool>("output_input"))
    T::_advanced_output_on["input"].clear();
}

template<class T>
AdvancedOutput<T>::~AdvancedOutput()
{
}

template<class T>
void
AdvancedOutput<T>::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for the output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for this output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for this output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not support for this output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for this output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputSystemInformation()
{
  mooseError("Output of system information is not support for this output object named '" << T::_name << "'");
}

template<class T>
void
AdvancedOutput<T>::outputInput()
{
  mooseError("Output of the input file information is not support for this output object named '" << T::_name << "'");
}

// General outputStep() method
template<class T>
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
template<>
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
template<>
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

template<class T>
void
AdvancedOutput<T>::output(const ExecFlagType & type)
{
  // Call the various output types, if data exists
  if (shouldOutput("nodal", type))
  {
    outputNodalVariables();
    _last_output_time["nodal"] = T::_time;
  }

  if (shouldOutput("elemental", type))
  {
    outputElementalVariables();
    _last_output_time["elemental"] = T::_time;
  }

  if (shouldOutput("postprocessors", type))
  {
    outputPostprocessors();
    _last_output_time["postprocessors"] = T::_time;
  }

  if (shouldOutput("vector_postprocessors", type))
  {
    outputVectorPostprocessors();
    _last_output_time["vector_postprocessors"] = T::_time;
  }

  if (shouldOutput("scalars", type))
  {
    outputScalarVariables();
    _last_output_time["scalars"] = T::_time;
  }

  if (shouldOutput("system_information", type))
  {
    outputSystemInformation();
    _last_output_time["system_information"] = T::_time;
  }

  if (shouldOutput("input", type))
  {
    outputInput();
    _last_output_time["input"] = T::_time;
  }
}

template<class T>
bool
AdvancedOutput<T>::shouldOutput(const std::string & name, const ExecFlagType & type)
{
  // Ignore EXEC_FORCED for system information and input, there is no reason to force this
  if (type == EXEC_FORCED && (name == "system_information" || name == "input"))
    return false;

  // Do not output if the 'none' is contained by the output_on
  if (T::_advanced_output_on.contains(name) && T::_advanced_output_on[name].contains("none"))
    return false;

  // Data output flag, true if data exists to be output
  bool output_data_flag = true;

  // Set flag to false, if the OutputData exists and the output variable list is empty
  std::map<std::string, OutputData>::const_iterator iter = _output_data.find(name);
  if (iter != _output_data.end() && iter->second.output.empty())
    output_data_flag = false;

  // Set flag to false, if the OutputOnWarehouse DOES NOT contain an entry
  if (!T::_advanced_output_on.contains(name))
    output_data_flag = false;

  // Force the output, if there is something to output and the time has not been output
  if (type == EXEC_FORCED && output_data_flag && _last_output_time[name] != T::_time)
    return true;

  // Return true (output should occur) if three criteria are satisfied, else do not output:
  //   (1) The output_data_flag = true (i.e, there is data to output)
  //   (2) The current output type is contained in the list of output execution types
  //   (3) The current execution time is "final" or "forced" and the data has not already been output
  if (output_data_flag && T::_advanced_output_on[name].contains(type) &&
      !(type == EXEC_FINAL && _last_output_time[name] == T::_time))
    return true;
  else
    return false;
}

template<class T>
bool
AdvancedOutput<T>::hasOutput(const ExecFlagType & type)
{
  // If any of the component outputs are true, then there is some output to perform
  for (std::map<std::string, MultiMooseEnum>::const_iterator it = T::_advanced_output_on.begin(); it != T::_advanced_output_on.end(); ++it)
    if (shouldOutput(it->first, type))
      return true;

  // There is nothing to output
  return false;
}

template<class T>
bool
AdvancedOutput<T>::hasOutput()
{
  // Test that variables exist for output AND that output execution flags are valid
  for (std::map<std::string, OutputData>::const_iterator it = _output_data.begin(); it != _output_data.end(); ++it)
    if (!(it->second).output.empty() &&
        T::_advanced_output_on.contains(it->first) &&
        T::_advanced_output_on[it->first].isValid())
      return true;

  // Test execution flags for non-variable output
  if (T::_advanced_output_on.contains("system_information") && T::_advanced_output_on["system_information"].isValid())
    return true;
  if (T::_advanced_output_on.contains("input") && T::_advanced_output_on["input"].isValid())
    return true;

  return false;
}

template<class T>
void
AdvancedOutput<T>::initAvailableLists()
{
  // Initialize Postprocessor list
  // This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if postprocessor output is disabled
  ExecStore<PostprocessorWarehouse> & warehouse = T::_problem_ptr->getPostprocessorWarehouse();
  initPostprocessorOrVectorPostprocessorLists<ExecStore<PostprocessorWarehouse>, Postprocessor>("postprocessors", warehouse);

  // Initialize vector postprocessor list
  // This flag is set to true if any vector postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if vector postprocessor output is disabled
  ExecStore<VectorPostprocessorWarehouse> & vector_warehouse = T::_problem_ptr->getVectorPostprocessorWarehouse();
  initPostprocessorOrVectorPostprocessorLists<ExecStore<VectorPostprocessorWarehouse>, VectorPostprocessor>("vector_postprocessors", vector_warehouse);

  // Get a list of the available variables
  std::vector<VariableName> variables = T::_problem_ptr->getVariableNames();

  // Loop through the variables and store the names in the correct available lists
  for (std::vector<VariableName>::const_iterator it = variables.begin(); it != variables.end(); ++it)
  {
    if (T::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].available.insert(*it);
      else
        _output_data["nodal"].available.insert(*it);
    }

    else if (T::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].available.insert(*it);
  }
}

template<class T>
void
AdvancedOutput<T>::initExecutionTypes(const std::string & name, MultiMooseEnum & input)
{
  // Build the input paramter name
  std::string param_name = "output_";
  param_name += name + "_on";

  // The parameters exists and has been set by the user
  if (T::_pars.template have_parameter<MultiMooseEnum>(param_name) && T::isParamValid(param_name))
  {
    input = T::template getParam<MultiMooseEnum>(param_name);

    if (name != "system_information" && name != "input")
      T::applyOutputOnShortCutFlags(input);
  }

  // If the parameter does not exists; set it to a state where no valid entries exists so nothing gets executed
  else if (!T::_pars. template have_parameter<MultiMooseEnum>(param_name))
    input = AdvancedOutput<T>::getExecuteOptions();

}

template<class T>
void
AdvancedOutput<T>::initShowHideLists(const std::vector<VariableName> & show, const std::vector<VariableName> & hide)
{

  // Storage for user-supplied input that is unknown as a variable or postprocessor
  std::set<std::string> unknown;

  // Populate the show lists
  for (std::vector<VariableName>::const_iterator it = show.begin(); it != show.end(); ++it)
  {
    if (T::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].show.insert(*it);
      else
        _output_data["nodal"].show.insert(*it);
    }
    else if (T::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].show.insert(*it);
    else if (T::_problem_ptr->hasPostprocessor(*it))
      _output_data["postprocessors"].show.insert(*it);
    else if (T::_problem_ptr->hasVectorPostprocessor(*it))
      _output_data["vector_postprocessors"].show.insert(*it);
    else
      unknown.insert(*it);
  }

  // Populate the hide lists
  for (std::vector<VariableName>::const_iterator it = hide.begin(); it != hide.end(); ++it)
  {
    if (T::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = T::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].hide.insert(*it);
      else
        _output_data["nodal"].hide.insert(*it);
    }
    else if (T::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].hide.insert(*it);
    else if (T::_problem_ptr->hasPostprocessor(*it))
      _output_data["postprocessors"].hide.insert(*it);
    else if (T::_problem_ptr->hasVectorPostprocessor(*it))
      _output_data["vector_postprocessors"].hide.insert(*it);
    else
      unknown.insert(*it);
  }

  // Error if an unknown variable or postprocessor is found
  if (!unknown.empty())
  {
    std::ostringstream oss;
    oss << "Output(s) do not exist (must be variable, scalar, postprocessor, or vector postprocessor): ";
    std::copy(unknown.begin(), unknown.end(), infix_ostream_iterator<std::string>(oss, " "));
    mooseError(oss.str());
  }
}

template<class T>
void
AdvancedOutput<T>::initOutputList(OutputData & data)
{
  // References to the vectors of variable names
  std::set<std::string> & hide  = data.hide;
  std::set<std::string> & show  = data.show;
  std::set<std::string> & avail = data.available;
  std::set<std::string> & output = data.output;

  // Append the list from OutputInterface objects
  std::set<std::string> interface_hide;
  T::_app.getOutputWarehouse().buildInterfaceHideVariables(T::_name, interface_hide);
  hide.insert(interface_hide.begin(), interface_hide.end());

  // Both show and hide are empty (show all available)
  if (show.empty() && hide.empty())
    output = avail;

  // Only hide is empty (show all the variables listed)
  else if (!show.empty() && hide.empty())
    output = show;

  // Only show is empty (show all except those hidden)
  else if (show.empty() && !hide.empty())
    std::set_difference(avail.begin(), avail.end(), hide.begin(), hide.end(), std::inserter(output, output.begin()));

  // Both hide and show are present (show all those listed)
  else
  {
    // Check if variables are in both, which is invalid
    std::vector<std::string> tmp;
    std::set_intersection(hide.begin(), hide.end(), avail.begin(), avail.end(), std::inserter(tmp, tmp.begin()));
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

template<class T>
void
AdvancedOutput<T>::addValidParams(InputParameters & params, const MultiMooseEnum & types)
{

  // Nodal output
  if (types.contains("nodal"))
    params.addParam<MultiMooseEnum>("output_nodal_on", T::getExecuteOptions(), "Control the output of nodal variables");

  // Elemental output
  if (types.contains("elemental"))
  {
    params.addParam<MultiMooseEnum>("output_elemental_on", T::getExecuteOptions(), "Control the output of elemental variables");

    // Add material output control, which are output via elemental variables
    params.addParam<bool>("output_material_properties", false, "Flag indicating if material properties should be output");
    params.addParam<std::vector<std::string> >("show_material_properties", "List of materialproperties that should be written to the output");
    params.addParamNamesToGroup("output_material_properties show_material_properties", "Materials");
    params.addParamNamesToGroup("show_material_properties", "Materials");
  }

  // Scalar variable output
  if (types.contains("scalar"))
    params.addParam<MultiMooseEnum>("output_scalars_on", T::getExecuteOptions(), "Control the output of scalar variables");

  // Nodal and scalar output
  if (types.contains("nodal") && types.contains("scalar"))
    params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");

  // Elemental and nodal
  if (types.contains("elemental") && types.contains("nodal"))
    params.addParam<bool>("elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");

  // Postprocessors
  if (types.contains("postprocessor"))
    params.addParam<MultiMooseEnum>("output_postprocessors_on", T::getExecuteOptions(), "Control of when postprocessors are output");
  // Vector Postprocessors
  if (types.contains("vector_postprocessor"))
    params.addParam<MultiMooseEnum>("output_vector_postprocessors_on", T::getExecuteOptions(), "Enable/disable the output of VectorPostprocessors");

  // Input file
  if (types.contains("input"))
    params.addParam<MultiMooseEnum>("output_input_on", T::getExecuteOptions(), "Enable/disable the output of the input file");

  // System Information
  if (types.contains("system_information"))
    params.addParam<MultiMooseEnum>("output_system_information_on", T::getExecuteOptions(), "Control when the output of the simulation information occurs");

  // Store everything in the 'Variables' group
  params.addParamNamesToGroup("scalar_as_nodal elemental_as_nodal output_scalars_on output_nodal_on output_elemental_on output_postprocessors_on output_vector_postprocessors_on output_system_information_on output_input_on", "Variables");
}

template<class T>
bool
AdvancedOutput<T>::hasOutputHelper(const std::string & name)
{
  if (!T::_initialized)
    mooseError("The output object must be initialized before it may be determined if " << name << " output is enabled.");

  return !_output_data[name].output.empty() && T::_advanced_output_on.contains(name) && T::_advanced_output_on[name].isValid() && !T::_advanced_output_on[name].contains("none");
}

template<class T>
bool
AdvancedOutput<T>::hasNodalVariableOutput()
{
  return hasOutputHelper("nodal");
}

template<class T>
const std::set<std::string> &
AdvancedOutput<T>::getNodalVariableOutput()
{
  return _output_data["nodal"].output;
}

template<class T>
bool
AdvancedOutput<T>::hasElementalVariableOutput()
{
  return hasOutputHelper("elemental");
}

template<class T>
const std::set<std::string> &
AdvancedOutput<T>::getElementalVariableOutput()
{
  return _output_data["elemental"].output;
}

template<class T>
bool
AdvancedOutput<T>::hasScalarOutput()
{
  return hasOutputHelper("scalars");
}

template<class T>
const std::set<std::string> &
AdvancedOutput<T>::getScalarOutput()
{
  return _output_data["scalars"].output;
}

template<class T>
bool
AdvancedOutput<T>::hasPostprocessorOutput()
{
  return hasOutputHelper("postprocessors");
}

template<class T>
const std::set<std::string> &
AdvancedOutput<T>::getPostprocessorOutput()
{
  return _output_data["postprocessors"].output;
}

template<class T>
bool
AdvancedOutput<T>::hasVectorPostprocessorOutput()
{
  return hasOutputHelper("vector_postprocessors");
}

template<class T>
const std::set<std::string> &
AdvancedOutput<T>::getVectorPostprocessorOutput()
{
  return _output_data["vector_postprocessors"].output;
}

template<class T>
const OutputOnWarehouse &
AdvancedOutput<T>::advancedOutputOn() const
{
  return T::_advanced_output_on;
}

// Instantiate the four possible template classes
template class AdvancedOutput<Output>;
template class AdvancedOutput<PetscOutput>;
template class AdvancedOutput<FileOutput>;
template class AdvancedOutput<OversampleOutput>;
