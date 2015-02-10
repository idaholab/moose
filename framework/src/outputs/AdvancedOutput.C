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
#include "MooseApp.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"
#include "CoupledExecutioner.h"
#include "VectorPostprocessor.h"
#include "MooseUtils.h"


// Constructor of OutputOnWarehouse; initializes the MultiMooseEnums for all available output types
OutputOnWarehouse::OutputOnWarehouse(const MultiMooseEnum & output_on, const InputParameters & params) : OutputMapWrapper<MultiMooseEnum>()
{
  // Initialize each of the output_on settings for the various types of outputs
  if (params.have_parameter<MultiMooseEnum>("output_nodal_on"))
    _map.insert(std::make_pair("nodal", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_elemental_on"))
    _map.insert(std::make_pair("elemental", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_scalars_on"))
    _map.insert(std::make_pair("scalars", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_postprocessors_on"))
    _map.insert(std::make_pair("postprocessors", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_vector_postprocessors_on"))
    _map.insert(std::make_pair("vector_postprocessors", output_on));

  if (params.have_parameter<MultiMooseEnum>("output_input_on"))
    _map.insert(std::make_pair("input", Output::getExecuteOptions()));

  if (params.have_parameter<MultiMooseEnum>("output_system_information_on"))
    _map.insert(std::make_pair("system_information", Output::getExecuteOptions("initial")));
}

// Constructor of OutputDataWarehouse; initializes the OutputData structures for 'variable' based output types
OutputDataWarehouse::OutputDataWarehouse() : OutputMapWrapper<OutputData>()
{
  _map["nodal"] = OutputData();
  _map["elemental"] = OutputData();
  _map["scalars"] = OutputData();
  _map["postprocessors"] = OutputData();
  _map["vector_postprocessors"] = OutputData();
}

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
template<class OutputBase>
MultiMooseEnum
AdvancedOutput<OutputBase>::getOutputTypes()
{
  return MultiMooseEnum("nodal=0 elemental=1 scalar=2 postprocessor=3 vector_postprocessor=4 input=5 system_information=6");
}

// Enables the output types (see getOutputTypes) for an AdvancedOutput object
template<class OutputBase>
InputParameters
AdvancedOutput<OutputBase>::enableOutputTypes(const std::string & names)
{
  // The parameters object that will be returned
  InputParameters params = emptyInputParameters();

  // Set private parameter indicating that this method was called
  params.addPrivateParam("_output_valid_params_was_called", true);

  // Get the MultiEnum of output types
  MultiMooseEnum output_types = AdvancedOutput<OutputBase>::getOutputTypes();

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
template<class OutputBase>
AdvancedOutput<OutputBase>::AdvancedOutput(const std::string & name, InputParameters & parameters) :
    OutputBase(name, parameters),
    _advanced_output_on(OutputBase::_output_on, parameters)
{
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::init()
{
  // Do not initialize more than once
  // This check is needed for YAK which calls Executioners from within Executioners
  if (OutputBase::_initialized)
    return;

  // Check that enable[disable]OutputTypes was called
  if (!OutputBase::isParamValid("_output_valid_params_was_called"))
    mooseError("The static method AdvancedOutput<OutputBase>::enableOutputTypes must be called inside the validParams function for this object to properly define the input parameters for the output object named '" << OutputBase::_name << "'");

  // Initialize the available output
  initAvailableLists();

  // Separate the hide/show list into components
  initShowHideLists(OutputBase::template getParam<std::vector<VariableName> >("show"),
                    OutputBase::template getParam<std::vector<VariableName> >("hide"));

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (OutputBase::isParamValid("elemental_as_nodal") && OutputBase::template getParam<bool>("elemental_as_nodal"))
  {
    OutputData & nodal = _output_data["nodal"];
    OutputData & elemental = _output_data["elemental"];
    nodal.show.insert(nodal.show.end(), elemental.show.begin(), elemental.show.end());
    nodal.hide.insert(nodal.hide.end(), elemental.hide.begin(), elemental.hide.end());
    nodal.available.insert(nodal.available.end(), elemental.available.begin(), elemental.available.end());
  }

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable lists
  if (OutputBase::isParamValid("scalar_as_nodal") && OutputBase::template getParam<bool>("scalar_as_nodal"))
  {
    OutputData & nodal = _output_data["nodal"];
    OutputData & scalar = _output_data["scalars"];
    nodal.show.insert(nodal.show.end(), scalar.show.begin(), scalar.show.end());
    nodal.hide.insert(nodal.hide.end(), scalar.hide.begin(), scalar.hide.end());
    nodal.available.insert(nodal.available.end(), scalar.available.begin(), scalar.available.end());
  }

  // Initialize the show/hide/output lists for each of the types of output
  for (std::map<std::string, OutputData>::iterator it = _output_data.begin(); it != _output_data.end(); ++it)
    initOutputList(it->second);

  // Initialize the execution flags
  for (std::map<std::string, MultiMooseEnum>::iterator it = _advanced_output_on.begin(); it != _advanced_output_on.end(); ++it)
    initExecutionTypes(it->first, it->second);

  // Set the initialization flag
  OutputBase::_initialized = true;

  // **** DEPRECATED PARAMETER SUPPORT ****
  if (OutputBase::isParamValid("output_postprocessors") && !OutputBase::template getParam<bool>("output_postprocessors"))
    _advanced_output_on["postprocessors"].clear();
  if (OutputBase::isParamValid("output_vector_postprocessors") && !OutputBase::template getParam<bool>("output_vector_postprocessors"))
    _advanced_output_on["vector_postprocessors"].clear();
  if (OutputBase::isParamValid("output_scalar_variables") && !OutputBase::template getParam<bool>("output_scalar_variables"))
    _advanced_output_on["scalars"].clear();
  if (OutputBase::isParamValid("output_elemental_variables") && !OutputBase::template getParam<bool>("output_elemental_variables"))
    _advanced_output_on["elemental"].clear();
  if (OutputBase::isParamValid("output_nodal_variables") && !OutputBase::template getParam<bool>("output_nodal_variables"))
    _advanced_output_on["nodal"].clear();
  if (OutputBase::isParamValid("output_system_information") && !OutputBase::template getParam<bool>("output_system_information"))
    _advanced_output_on["system_information"].clear();
  if (OutputBase::isParamValid("output_input") && !OutputBase::template getParam<bool>("output_input"))
    _advanced_output_on["input"].clear();
}

template<class OutputBase>
AdvancedOutput<OutputBase>::~AdvancedOutput()
{
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for the output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for this output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for this output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not support for this output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for this output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputSystemInformation()
{
  mooseError("Output of system information is not support for this output object named '" << OutputBase::_name << "'");
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputInput()
{
  mooseError("Output of the input file information is not support for this output object named '" << OutputBase::_name << "'");
}

// General outputStep() method
template<class OutputBase>
void
AdvancedOutput<OutputBase>::outputStep(const ExecFlagType & type)
{

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && OutputBase::_app.isRecovering())
    return;

  // Return if the current output is not on the desired interval
  if (type != EXEC_FINAL && !OutputBase::onInterval())
    return;

  // Call output methods for various types
  output(type);
}


// FileOutput::outputStep specialization
template<>
void
AdvancedOutput<FileOutput>::outputStep(const ExecFlagType & type)
{

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

template<class OutputBase>
void
AdvancedOutput<OutputBase>::output(const ExecFlagType & type)
{
  // Call the various output types, if data exists
  if (shouldOutput("nodal", type))
  {
    outputNodalVariables();
    _last_output_time["nodal"] = OutputBase::_time;
  }

  if (shouldOutput("elemental", type))
  {
    outputElementalVariables();
    _last_output_time["elemental"] = OutputBase::_time;
  }

  if (shouldOutput("postprocessors", type))
  {
    outputPostprocessors();
    _last_output_time["postprocessors"] = OutputBase::_time;
  }

  if (shouldOutput("vector_postprocessors", type))
  {
    outputVectorPostprocessors();
    _last_output_time["vector_postprocessors"] = OutputBase::_time;
  }

  if (shouldOutput("scalars", type))
  {
    outputScalarVariables();
    _last_output_time["scalars"] = OutputBase::_time;
  }

  if (shouldOutput("system_information", type))
  {
    outputSystemInformation();
    _last_output_time["system_information"] = OutputBase::_time;
  }

  if (shouldOutput("input", type))
  {
    outputInput();
    _last_output_time["input"] = OutputBase::_time;
  }
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::shouldOutput(const std::string & name, const ExecFlagType & type)
{
  // Ignore EXEC_FORCED for system information and input, there is no reason to force this
  if (type == EXEC_FORCED && (name == "system_information" || name == "input"))
    return false;

  // Do not output if the 'none' is contained by the output_on
  if (_advanced_output_on.contains(name) && _advanced_output_on[name].contains("none"))
    return false;

  // Data output flag, true if data exists to be output
  bool output_data_flag = true;

  // Set flag to false, if the OutputData exists and the output variable list is empty
  std::map<std::string, OutputData>::const_iterator iter = _output_data.find(name);
  if (iter != _output_data.end() && iter->second.output.empty())
    output_data_flag = false;

  // Set flag to false, if the OutputOnWarehouse DOES NOT contain an entry
  if (!_advanced_output_on.contains(name))
    output_data_flag = false;

  // Force the output, if there is something to output and the time has not been output
  if (type == EXEC_FORCED && output_data_flag && _last_output_time[name] != OutputBase::_time)
    return true;

  // Return true (output should occur) if three criteria are satisfied, else do not output:
  //   (1) The output_data_flag = true (i.e, there is data to output)
  //   (2) The current output type is contained in the list of output execution types
  //   (3) The current execution time is "final" or "forced" and the data has not already been output
  if (output_data_flag && _advanced_output_on[name].contains(type) &&
      !(type == EXEC_FINAL && _last_output_time[name] == OutputBase::_time))
    return true;
  else
    return false;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasOutput(const ExecFlagType & type)
{
  // If any of the component outputs are true, then there is some output to perform
  for (std::map<std::string, MultiMooseEnum>::const_iterator it = _advanced_output_on.begin(); it != _advanced_output_on.end(); ++it)
    if (shouldOutput(it->first, type))
      return true;

  // There is nothing to output
  return false;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasOutput()
{
  // Test that variables exist for output AND that output execution flags are valid
  for (std::map<std::string, OutputData>::const_iterator it = _output_data.begin(); it != _output_data.end(); ++it)
    if (!(it->second).output.empty() &&
        _advanced_output_on.contains(it->first) &&
        _advanced_output_on[it->first].isValid())
      return true;

  // Test execution flags for non-variable output
  if (_advanced_output_on.contains("system_information") && _advanced_output_on["system_information"].isValid())
    return true;
  if (_advanced_output_on.contains("input") && _advanced_output_on["input"].isValid())
    return true;

  return false;
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::initAvailableLists()
{
  // Initialize Postprocessor list
  // This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if postprocessor output is disabled
  ExecStore<PostprocessorWarehouse> & warehouse = OutputBase::_problem_ptr->getPostprocessorWarehouse();
  initPostprocessorOrVectorPostprocessorLists<ExecStore<PostprocessorWarehouse>, Postprocessor>("postprocessors", warehouse);

  // Initialize vector postprocessor list
  // This flag is set to true if any vector postprocessor has the 'outputs' parameter set, it is then used
  // to produce an warning if vector postprocessor output is disabled
  ExecStore<VectorPostprocessorWarehouse> & vector_warehouse = OutputBase::_problem_ptr->getVectorPostprocessorWarehouse();
  initPostprocessorOrVectorPostprocessorLists<ExecStore<VectorPostprocessorWarehouse>, VectorPostprocessor>("vector_postprocessors", vector_warehouse);

  // Get a list of the available variables
  std::vector<VariableName> variables = OutputBase::_problem_ptr->getVariableNames();

  // Loop through the variables and store the names in the correct available lists
  for (std::vector<VariableName>::const_iterator it = variables.begin(); it != variables.end(); ++it)
  {
    if (OutputBase::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = OutputBase::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].available.push_back(*it);
      else
        _output_data["nodal"].available.push_back(*it);
    }

    else if (OutputBase::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].available.push_back(*it);
  }
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::initExecutionTypes(const std::string & name, MultiMooseEnum & input)
{
  // Build the input paramter name
  std::string param_name = "output_";
  param_name += name + "_on";

  // The parameters exists and has been set by the user
  if (OutputBase::_pars.template have_parameter<MultiMooseEnum>(param_name) && OutputBase::isParamValid(param_name))
    input = OutputBase::template getParam<MultiMooseEnum>(param_name);

  // If the parameter does not exists; set it to a state where no valid entires exists so nothing gets executed
  else if (!OutputBase::_pars. template have_parameter<MultiMooseEnum>(param_name))
    input = AdvancedOutput<OutputBase>::getExecuteOptions();
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::initShowHideLists(const std::vector<VariableName> & show, const std::vector<VariableName> & hide)
{

  // Storage for user-supplied input that is unknown as a variable or postprocessor
  std::vector<std::string> unknown;

  // Populate the show lists
  for (std::vector<VariableName>::const_iterator it = show.begin(); it != show.end(); ++it)
  {
    if (OutputBase::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = OutputBase::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].show.push_back(*it);
      else
        _output_data["nodal"].show.push_back(*it);
    }
    else if (OutputBase::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].show.push_back(*it);
    else if (OutputBase::_problem_ptr->hasPostprocessor(*it))
      _output_data["postprocessors"].show.push_back(*it);
    else if (OutputBase::_problem_ptr->hasVectorPostprocessor(*it))
      _output_data["vector_postprocessors"].show.push_back(*it);
    else
      unknown.push_back(*it);
  }

  // Populate the hide lists
  for (std::vector<VariableName>::const_iterator it = hide.begin(); it != hide.end(); ++it)
  {
    if (OutputBase::_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = OutputBase::_problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _output_data["elemental"].hide.push_back(*it);
      else
        _output_data["nodal"].hide.push_back(*it);
    }
    else if (OutputBase::_problem_ptr->hasScalarVariable(*it))
      _output_data["scalars"].hide.push_back(*it);
    else if (OutputBase::_problem_ptr->hasPostprocessor(*it))
      _output_data["postprocessors"].hide.push_back(*it);
    else if (OutputBase::_problem_ptr->hasVectorPostprocessor(*it))
      _output_data["vector_postprocessors"].hide.push_back(*it);
    else
      unknown.push_back(*it);
  }

  // Error if an unknown variable or postprocessor is found
  if (!unknown.empty())
  {
    std::ostringstream oss;
    oss << "Output(s) do not exist (must be variable, scalar, postprocessor, or vector postprocessor): " << (*unknown.begin());
    for (std::vector<std::string>::iterator it = unknown.begin()+1; it != unknown.end();  ++it)
      oss << ", " << *it;
    mooseError(oss.str());
  }
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::initOutputList(OutputData & data)
{
  // References to the vectors of variable names
  std::vector<std::string> & hide  = data.hide;
  std::vector<std::string> & show  = data.show;
  std::vector<std::string> & avail = data.available;
  std::vector<std::string> & output = data.output;

  // Append the list from OutputInterface objects
  std::set<std::string> interface_hide;
  OutputBase::_app.getOutputWarehouse().buildInterfaceHideVariables(OutputBase::_name, interface_hide);
  hide.insert(hide.end(), interface_hide.begin(), interface_hide.end());

  // Sort the vectors
  std::sort(avail.begin(), avail.end());
  std::sort(show.begin(), show.end());
  std::sort(hide.begin(), hide.end());

  // Both show and hide are empty (show all available)
  if (show.empty() && hide.empty())
    output.assign(avail.begin(), avail.end());

  // Only hide is empty (show all the variables listed)
  else if (!show.empty() && hide.empty())
    output.assign(show.begin(), show.end());

  // Only show is empty (show all except those hidden)
  else if (show.empty() && !hide.empty())
    std::set_difference(avail.begin(), avail.end(), hide.begin(), hide.end(), std::back_inserter(output));

  // Both hide and show are present (show all those listed)
  else
  {
    // Check if variables are in both, which is invalid
    std::vector<std::string> tmp;
    std::set_intersection(hide.begin(), hide.end(), avail.begin(), avail.end(), std::back_inserter(tmp));
    if (!tmp.empty())
    {
      std::ostringstream oss;
      oss << "Output(s) specified to be both shown and hidden: " << (*tmp.begin());
      for (std::vector<std::string>::iterator it = tmp.begin()+1; it != tmp.end();  ++it)
        oss << ", " << *it;
      mooseError(oss.str());
    }

    // Define the output variable list
    output.assign(show.begin(), show.end());
  }
}

template<class OutputBase>
void
AdvancedOutput<OutputBase>::addValidParams(InputParameters & params, const MultiMooseEnum & types)
{

  // Nodal output
  if (types.contains("nodal"))
    params.addParam<MultiMooseEnum>("output_nodal_on", OutputBase::getExecuteOptions(), "Control the output of nodal variables");

  // Elemental output
  if (types.contains("elemental"))
  {
    params.addParam<MultiMooseEnum>("output_elemental_on", OutputBase::getExecuteOptions(), "Control the output of elemental variables");

    // Add material output control, which are output via elemental variables
    params.addParam<bool>("output_material_properties", false, "Flag indicating if material properties should be output");
    params.addParam<std::vector<std::string> >("show_material_properties", "List of materialproperties that should be written to the output");
    params.addParamNamesToGroup("output_material_properties show_material_properties", "Materials");
    params.addParamNamesToGroup("show_material_properties", "Materials");
  }

  // Scalar variable output
  if (types.contains("scalar"))
    params.addParam<MultiMooseEnum>("output_scalars_on", OutputBase::getExecuteOptions(), "Control the output of scalar variables");

  // Nodal and scalar output
  if (types.contains("nodal") && types.contains("scalar"))
    params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");

  // Elemental and nodal
  if (types.contains("elemental") && types.contains("nodal"))
    params.addParam<bool>("elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");

  // Postprocessors
  if (types.contains("postprocessor"))
    params.addParam<MultiMooseEnum>("output_postprocessors_on", OutputBase::getExecuteOptions(), "Control of when postprocessors are output");
  // Vector Postprocessors
  if (types.contains("vector_postprocessor"))
    params.addParam<MultiMooseEnum>("output_vector_postprocessors_on", OutputBase::getExecuteOptions(), "Enable/disable the output of VectorPostprocessors");

  // Input file
  if (types.contains("input"))
    params.addParam<MultiMooseEnum>("output_input_on", OutputBase::getExecuteOptions(), "Enable/disable the output of the input file");

  // System Information
  if (types.contains("system_information"))
    params.addParam<MultiMooseEnum>("output_system_information_on", OutputBase::getExecuteOptions(), "Control when the output of the simulation information occurs");

  // Store everything in the 'Variables' group
  params.addParamNamesToGroup("scalar_as_nodal elemental_as_nodal output_scalars_on output_nodal_on output_elemental_on output_postprocessors_on output_vector_postprocessors_on output_system_information_on output_input_on", "Variables");
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasOutputHelper(const std::string & name)
{
  if (!OutputBase::_initialized)
    mooseError("The output object must be initialized before it may be determined if " << name << " output is enabled.");

  return !_output_data[name].output.empty() && _advanced_output_on.contains(name) && _advanced_output_on[name].isValid() && !_advanced_output_on[name].contains("none");
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasNodalVariableOutput()
{
  return hasOutputHelper("nodal");
}

template<class OutputBase>
const std::vector<std::string> &
AdvancedOutput<OutputBase>::getNodalVariableOutput()
{
  return _output_data["nodal"].output;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasElementalVariableOutput()
{
  return hasOutputHelper("elemental");
}

template<class OutputBase>
const std::vector<std::string> &
AdvancedOutput<OutputBase>::getElementalVariableOutput()
{
  return _output_data["elemental"].output;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasScalarOutput()
{
  return hasOutputHelper("scalars");
}

template<class OutputBase>
const std::vector<std::string> &
AdvancedOutput<OutputBase>::getScalarOutput()
{
  return _output_data["scalars"].output;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasPostprocessorOutput()
{
  return hasOutputHelper("postprocessors");
}

template<class OutputBase>
const std::vector<std::string> &
AdvancedOutput<OutputBase>::getPostprocessorOutput()
{
  return _output_data["postprocessors"].output;
}

template<class OutputBase>
bool
AdvancedOutput<OutputBase>::hasVectorPostprocessorOutput()
{
  return hasOutputHelper("vector_postprocessors");
}

template<class OutputBase>
const std::vector<std::string> &
AdvancedOutput<OutputBase>::getVectorPostprocessorOutput()
{
  return _output_data["vector_postprocessors"].output;
}

// Instatiate the four possible template classes
template class AdvancedOutput<Output>;
template class AdvancedOutput<PetscOutput>;
template class AdvancedOutput<FileOutput>;
template class AdvancedOutput<OversampleOutput>;
