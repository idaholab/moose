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
#include "Output.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"
#include "CoupledExecutioner.h"
#include "VectorPostprocessor.h"

template<>
InputParameters validParams<Output>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<MooseObject>();

  // General options
  params.addParam<bool>("output_input", false, "Output the input file");
  params.addParam<bool>("output_system_information", true, "Toggles the display of the system information prior to the solve");

  // Hide/show variable output options
  params.addParam<std::vector<VariableName> >("hide", "A list of the variables and postprocessors that should NOT be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");
  params.addParam<std::vector<VariableName> >("show", "A list of the variables and postprocessors that should be output to the Exodus file (may include Variables, ScalarVariables, and Postprocessor names).");

  // Enable/disable output types
  params.addParam<bool>("output_nodal_variables", true, "Enable/disable the output of nodal nonlinear variables");
  params.addParam<bool>("output_elemental_variables", true, "Enable/disable the output of elemental nonlinear variables");
  params.addParam<bool>("output_scalar_variables", true, "Enable/disable the output of aux scalar variables");
  params.addParam<bool>("output_postprocessors", true, "Enable/disable the output of postprocessors");
  params.addParam<bool>("output_vector_postprocessors", true, "Enable/disable the output of VectorPostprocessors");

  // Displaced Mesh options
  params.addParam<bool>("use_displaced", false, "Enable/disable the use of the displaced mesh for outputting");

  // Enable sequential file output (do not set default, the use_displace criteria relies on isParamValid, see Constructor)
  params.addParam<bool>("sequence", "Enable/disable sequential file output (enable by default when 'use_displace = true', otherwise defaults to false");

  // Control for outputting elemental variables as nodal variables
  params.addParam<bool>("elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");
  params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");

  // Output intervals and timing
  params.addParam<bool>("output_initial", false, "Request that the initial condition is output to the solution file");
  params.addParam<bool>("output_intermediate", true, "Request that all intermediate steps (not initial or final) are output");
  params.addParam<bool>("output_final", false, "Force the final time step to be output, regardless of output interval");
  params.addParam<unsigned int>("interval", 1, "The interval at which time steps are output to the solution file");
  params.addParam<bool>("output_failed", false, "When true all time attempted time steps are output");
  params.addParam<std::vector<Real> >("sync_times", "Times at which the output and solution is forced to occur");
  params.addParam<bool>("sync_only", false, "Only export results at sync times");
  params.addParam<Real>("start_time", "Time at which this output object begins to operate");
  params.addParam<Real>("end_time", "Time at which this output object stop operating");
  params.addParam<Real>("time_tolerance", 1e-14, "Time tolerance utilized checking start and end times");

  // 'Timing' group
  params.addParamNamesToGroup("time_tolerance interval output_initial output_final sync_times sync_only start_time end_time ", "Timing");

  // 'Variables' Group
  params.addParamNamesToGroup("hide show output_nonlinear_variables output_postprocessors output_vector_postprocessors output_scalar_variables output_elemental_variables output_nodal_variables scalar_as_nodal elemental_as_nodal", "Variables");

  // Add a private parameter for indicating if it was created with short-cut syntax
  params.addPrivateParam<bool>("_built_by_moose", false);

  // Register this class as base class
  params.registerBase("Output");
  return params;
}

Output::Output(const std::string & name, InputParameters & parameters) :
    MooseObject(name, parameters),
    Restartable(name, parameters, "Output"),
    _problem_ptr(getParam<FEProblem *>("_fe_problem")),
    _transient(_problem_ptr->isTransient()),
    _use_displaced(getParam<bool>("use_displaced")),
    _es_ptr(_use_displaced ? &_problem_ptr->getDisplacedProblem()->es() : &_problem_ptr->es()),
    _output_initial(getParam<bool>("output_initial")),
    _output_intermediate(getParam<bool>("output_intermediate")),
    _output_final(getParam<bool>("output_final")),
    _output_input(getParam<bool>("output_input")),
    _elemental_as_nodal(getParam<bool>("elemental_as_nodal")),
    _scalar_as_nodal(getParam<bool>("scalar_as_nodal")),
    _system_information(getParam<bool>("output_system_information")),
    _mesh_changed(declareRestartableData<bool>("mesh_changed", false)),
    _sequence(declareRestartableData<bool>("sequence", isParamValid("sequence") ? getParam<bool>("sequence") : false)),
    _allow_output(true),
    _time(_problem_ptr->time()),
    _time_old(_problem_ptr->timeOld()),
    _t_step(_problem_ptr->timeStep()),
    _dt(_problem_ptr->dt()),
    _dt_old(_problem_ptr->dtOld()),
    _num(declareRestartableData<unsigned int>("num", 0)),
    _interval(getParam<unsigned int>("interval")),
    _sync_times(std::set<Real>(getParam<std::vector<Real> >("sync_times").begin(), getParam<std::vector<Real> >("sync_times").end())),
    _start_time(isParamValid("start_time") ? getParam<Real>("start_time") : -std::numeric_limits<Real>::max()),
    _end_time(isParamValid("end_time") ? getParam<Real>("end_time") : std::numeric_limits<Real>::max()),
    _t_tol(getParam<Real>("time_tolerance")),
    _sync_only(getParam<bool>("sync_only")),
    _force_output(false),
    _output_failed(false),
    _output_setup_called(false),
    _initialized(false),
    _on_initial(false)
{
}

void
Output::init()
{
  // Do not initialize more than once
  /* This check is needed for YAK which calls Executioners from within Executioners */
  if (_initialized)
    return;

  // If recovering disable output of initial condition to avoid duplicate files
  if (_app.isRecovering())
    _output_initial = false;

  // Set the sequence flag to true if it has not been set and 'use_displaced = true'
  if (!isParamValid("sequence") && _use_displaced)
    sequence(true);

  // Initialize the available output
  initAvailableLists();

  // Separate the hide/show list into components
  initShowHideLists(getParam<std::vector<VariableName> >("show"), getParam<std::vector<VariableName> >("hide"));

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (_elemental_as_nodal)
  {
    _nonlinear_nodal.show.insert(_nonlinear_nodal.show.end(), _nonlinear_elemental.show.begin(), _nonlinear_elemental.show.end());
    _nonlinear_nodal.hide.insert(_nonlinear_nodal.hide.end(), _nonlinear_elemental.hide.begin(), _nonlinear_elemental.hide.end());
    _nonlinear_nodal.available.insert(_nonlinear_nodal.available.end(), _nonlinear_elemental.available.begin(), _nonlinear_elemental.available.end());
  }

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable lists
  if (_scalar_as_nodal)
  {
    _nonlinear_nodal.show.insert(_nonlinear_nodal.show.end(), _scalar.show.begin(), _scalar.show.end());
    _nonlinear_nodal.hide.insert(_nonlinear_nodal.hide.end(), _scalar.hide.begin(), _scalar.hide.end());
    _nonlinear_nodal.available.insert(_nonlinear_nodal.available.end(), _scalar.available.begin(), _scalar.available.end());
  }

  // Initialize the show/hide/output lists for each of the types of output
  initOutputList(_nonlinear_nodal);
  initOutputList(_nonlinear_elemental);
  initOutputList(_scalar);
  initOutputList(_postprocessor);
  initOutputList(_vector_postprocessor);

  // Disable output lists based on two items:
  //   (1) If the toggle parameter is invalid
  //   (2) If the toggle is set to false
  // This is done after initialization to allow for the appending of the elemental output to occur, because
  // it is possible to output a scalar variable as a nodal, then disable the output of scalars, resulting
  // in only the nodal version of the scalar variable to be in the output file (Exodus supports this). The
  // same is true for elemental variables.
  if (isParamValid("output_elemental_variables") ? !getParam<bool>("output_elemental_variables") : true)
    _nonlinear_elemental.output.clear();

  if (isParamValid("output_nodal_variables") ? !getParam<bool>("output_nodal_variables") : true)
    _nonlinear_nodal.output.clear();

  if (isParamValid("output_scalar_variables") ? !getParam<bool>("output_scalar_variables") : true)
    _scalar.output.clear();

  if (isParamValid("output_postprocessors") ? !getParam<bool>("output_postprocessors") : true)
    _postprocessor.output.clear();

  if (isParamValid("output_vector_postprocessors") ? !getParam<bool>("output_vector_postprocessors") : true)
    _vector_postprocessor.output.clear();

  // Set the _output_input bool, this is done here rather than the constructor so that CheckOutputAction can force in case --show-input is used
  _output_input = getParam<bool>("output_input");

  // Set the initialization flag
  _initialized = true;

  // Assume after init()
  /* Between init() and the first call of outputStep() all output is considered to be a part of initial output. This is
     mainly only import for calls to _console that occur during this time */
  _on_initial = true;
}

Output::~Output()
{
}

void
Output::outputSetup()
{
}

void
Output::initialSetup()
{
}

void
Output::timestepSetup()
{
}

void
Output::timestepSetupInternal()
{
}

void
Output::outputInitial()
{
  if (shouldOutput())
  {
    outputSetup();
    _mesh_changed = false;
    _output_setup_called = true;
    output();
    _num++;
  }

  // Set the force output flag to false
  _force_output = false;

}

void
Output::outputFailedStep()
{
  if (_output_failed)
    outputStep();
}

void
Output::outputStep()
{
  // Set output initial related flags, outputting of initial is over at the first call to outputStep
  _on_initial = false;
  _output_initial = false;

  // Only perform output if you should
  if (!shouldOutput())
    return;

  // If the mesh has changed or the sequence state is true or if it has not been called, call the outputSetup() function
  if (_mesh_changed || _sequence || _num == 0 || !_output_setup_called)
  {
    // Execute the setup function
    outputSetup();

    // Reset the _mesh_changed flag
    _mesh_changed = false;

    // outputSetup has been called
    _output_setup_called = true;
  }

  // Update the output number
  _num++;

  // Perform the output
  output();

  // Set the force output flag to false
  _force_output = false;
}

bool
Output::shouldOutput()
{
  // Do Nothing if output is not forced and allowing output has been disabled
  if (!_force_output && !_allow_output)
    return false;

  // On initial step: between init() and first call to outputStep()
  if (onInitial())
  {
    if (_force_output || _output_initial)
      return true;
    else
      return false;
  }

  // On intermediate steps
  else
  {
    // Do not perform output if:
    // (1) Intermediate output is disabled
    // (2) Output is not forced and (output is not allowed or not on interval)
    if (!_output_intermediate || (!_force_output && (!_allow_output || !checkInterval())))
      return false;
    else
      return true;
  }
}

void
Output::outputFinal()
{
  // If the intermediate steps are being output and the final time step is on an interval it will already have been output by outputStep, so do nothing
  if (checkInterval() && _output_intermediate)
    return;

  // Do nothing if output is not forced or if output is disallowed
  if (!_force_output && !_allow_output)
    return;

  // Do nothing if the output is not forced and final output is not desired
  if (!_force_output && !_output_final)
    return;

  // If the mesh has changed or the sequence state is true, call the outputSetup() function
  if (_mesh_changed || _sequence || !_output_setup_called)
  {
    outputSetup();
    _output_setup_called = true;
  }

  // Perform the output
  output();

  // Set the force output flag to false
  _force_output = false;
}

void
Output::output()
{
  // Call the various output types, if data exists
  if (hasNodalVariableOutput())
    outputNodalVariables();

  if (hasElementalVariableOutput())
    outputElementalVariables();

  if (hasPostprocessorOutput())
    outputPostprocessors();

  if (hasVectorPostprocessorOutput())
    outputVectorPostprocessors();

  if (hasScalarOutput())
    outputScalarVariables();
}

void
Output::forceOutput()
{
  _force_output = true;
}

bool
Output::hasOutput()
{
  // Test all the possible output formats, return true if any of them are true
  if (hasNodalVariableOutput() || hasElementalVariableOutput() ||
      hasScalarOutput() || hasPostprocessorOutput() || hasVectorPostprocessorOutput())
    return true;
  else
    return false;
}

void
Output::outputInput()
{
  // The method left intentially empty
}

void
Output::outputSystemInformation()
{
  // Empty function
}

bool
Output::hasNodalVariableOutput()
{
  return !_nonlinear_nodal.output.empty();
}

const std::vector<std::string> &
Output::getNodalVariableOutput()
{
  return _nonlinear_nodal.output;
}

bool
Output::hasElementalVariableOutput()
{
  return !_nonlinear_elemental.output.empty();
}

const std::vector<std::string> &
Output::getElementalVariableOutput()
{
  return _nonlinear_elemental.output;
}

bool
Output::hasScalarOutput()
{
  return !_scalar.output.empty();
}

const std::vector<std::string> &
Output::getScalarOutput()
{
  return _scalar.output;
}

bool
Output::hasPostprocessorOutput()
{
  return !_postprocessor.output.empty();
}

const std::vector<std::string> &
Output::getPostprocessorOutput()
{
  return _postprocessor.output;
}

bool
Output::hasVectorPostprocessorOutput()
{
  return !_vector_postprocessor.output.empty();
}

const std::vector<std::string> &
Output::getVectorPostprocessorOutput()
{
  return _vector_postprocessor.output;
}

void
Output::meshChanged()
{
  _mesh_changed = true;
}

void
Output::allowOutput(bool state)
{
  _allow_output = state;
}


void
Output::sequence(bool state)
{
  _sequence = state;
}

bool
Output::onInitial()
{
  return _on_initial;
}

bool
Output::checkInterval()
{
  // The output flag to return
  bool output = false;

  // Return true if the current step on the current output interval and within the output time range
  if (_time >= _start_time && _time <= _end_time && (_t_step % _interval) == 0 )
    output = true;

  // Return false if 'sync_only' is set to true
  if (_sync_only)
    output = false;

  // If sync times are not skipped, return true if the current time is a sync_time
  if (_sync_times.find(_time) != _sync_times.end())
    output = true;

  // Return the output status
  return output;
}

// Helper function for initAvailableLists, templated on warehouse type and postprocessor_type
template <typename warehouse_type, typename postprocessor_type>
void
initPostprocessorOrVectorPostprocessorLists(OutputData & output_data, warehouse_type & warehouse, bool & has_limited_pps, MooseApp & app, std::string & name, InputParameters & params)
{
  // Loop through each of the execution flags
  for (unsigned int i = 0; i < Moose::exec_types.size(); i++)
  {
    // Loop through each of the postprocessors
    for (typename std::vector<postprocessor_type *>::const_iterator postprocessor_it = warehouse(Moose::exec_types[i])[0].all().begin();
         postprocessor_it != warehouse(Moose::exec_types[i])[0].all().end();
         ++postprocessor_it)
    {
      // Store the name in the available postprocessors
      postprocessor_type *pps = *postprocessor_it;
      output_data.available.push_back(pps->PPName());

      // Extract the list of outputs
      std::set<OutputName> pps_outputs = pps->getOutputs();

      // Check that the outputs are valid
      app.getOutputWarehouse().checkOutputs(pps_outputs);

      /* Hide the postprocessor if:
       *  (1) The "outputs" parameter is NOT empty and
       *  (2) 'all' is NOT found in the 'outputs' parameter and
       *  (3) 'none' is used within the 'outputs' parameter or
       *  (4) this output object name is not found in the list of output names
       */
      if ( !pps_outputs.empty() && pps_outputs.find("all") == pps_outputs.end() &&
           (pps_outputs.find("none") != pps_outputs.end() || pps_outputs.find(name) == pps_outputs.end()) )
        output_data.hide.push_back(pps->PPName());

      // Check that the output object allows postprocessor output, account for "all" keyword (if it is present assume "all" was desired)
      if ( pps_outputs.find(name) != pps_outputs.end() || pps_outputs.find("all") != pps_outputs.end() )
      {
        if (!params.isParamValid("output_postprocessors"))
          mooseWarning("Postprocessor '" << pps->PPName()
                       << "' has requested to be output by the '" << name
                       << "' output, but postprocessor output is not support by this type of output object.");
      }

      // Set the flag state for postprocessors that utilize 'outputs' parameter
      if (!pps_outputs.empty() && pps_outputs.find("all") == pps_outputs.end())
        has_limited_pps = true;
    }
  }
}


void
Output::initAvailableLists()
{
  /* This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
     to produce an warning if postprocessor output is disabled*/
  bool has_limited_pps = false;

  /* This flag is set to true if any vector postprocessor has the 'outputs' parameter set, it is then used
     to produce an warning if postprocessor output is disabled*/
  bool has_limited_vector_pps = false;

  {
    // Get a reference to the storage of the postprocessors
    ExecStore<PostprocessorWarehouse> & warehouse = _problem_ptr->getPostprocessorWarehouse();

    initPostprocessorOrVectorPostprocessorLists<ExecStore<PostprocessorWarehouse>, Postprocessor>(_postprocessor, warehouse, has_limited_pps, _app, _name, _pars);
  }

  {
    // Get a reference to the storage of the vector postprocessors
    ExecStore<VectorPostprocessorWarehouse> & warehouse = _problem_ptr->getVectorPostprocessorWarehouse();

    initPostprocessorOrVectorPostprocessorLists<ExecStore<VectorPostprocessorWarehouse>, VectorPostprocessor>(_vector_postprocessor, warehouse, has_limited_vector_pps, _app, _name, _pars);
  }


  // Produce the warning when 'outputs' is used, but postprocessor output is disable
  if (has_limited_pps && isParamValid("output_postprocessors") && getParam<bool>("output_postprocessors") == false)
    mooseWarning("A Postprocessor utilizes the 'outputs' parameter; however, postprocessor output is disabled for the '" << _name << "' output object.");

  // Produce the warning when 'outputs' is used, but postprocessor output is disable
  if (has_limited_vector_pps && isParamValid("output_vector_postprocessors") && getParam<bool>("output_vector_postprocessors") == false)
    mooseWarning("A VectorPostprocessor utilizes the 'outputs' parameter; however, vector postprocessor output is disabled for the '" << _name << "' output object.");

  // Get a list of the available variables
  std::vector<VariableName> variables = _problem_ptr->getVariableNames();

  // Loop through the variables and store the names in the correct available lists
  for (std::vector<VariableName>::const_iterator it = variables.begin(); it != variables.end(); ++it)
  {
    if (_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = _problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _nonlinear_elemental.available.push_back(*it);
      else
        _nonlinear_nodal.available.push_back(*it);
    }

    else if (_problem_ptr->hasScalarVariable(*it))
      _scalar.available.push_back(*it);
  }
}

void
Output::initShowHideLists(const std::vector<VariableName> & show, const std::vector<VariableName> & hide)
{

  // Storage for user-supplied input that is unknown as a variable or postprocessor
  std::vector<std::string> unknown;

  // Populate the show lists
  for (std::vector<VariableName>::const_iterator it = show.begin(); it != show.end(); ++it)
  {
    if (_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = _problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _nonlinear_elemental.show.push_back(*it);
      else
        _nonlinear_nodal.show.push_back(*it);
    }
    else if (_problem_ptr->hasScalarVariable(*it))
      _scalar.show.push_back(*it);
    else if (_problem_ptr->hasPostprocessor(*it))
      _postprocessor.show.push_back(*it);
    else if (_problem_ptr->hasVectorPostprocessor(*it))
      _vector_postprocessor.show.push_back(*it);
    else
      unknown.push_back(*it);
  }

  // Populate the hide lists
  for (std::vector<VariableName>::const_iterator it = hide.begin(); it != hide.end(); ++it)
  {
    if (_problem_ptr->hasVariable(*it))
    {
      MooseVariable & var = _problem_ptr->getVariable(0, *it);
      const FEType type = var.feType();
      if (type.order == CONSTANT)
        _nonlinear_elemental.hide.push_back(*it);
      else
        _nonlinear_nodal.hide.push_back(*it);
    }
    else if (_problem_ptr->hasScalarVariable(*it))
      _scalar.hide.push_back(*it);
    else if (_problem_ptr->hasPostprocessor(*it))
      _postprocessor.hide.push_back(*it);
    else if (_problem_ptr->hasVectorPostprocessor(*it))
      _vector_postprocessor.hide.push_back(*it);
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

void
Output::initOutputList(OutputData & data)
{
  // References to the vectors of variable names
  std::vector<std::string> & hide  = data.hide;
  std::vector<std::string> & show  = data.show;
  std::vector<std::string> & avail = data.available;
  std::vector<std::string> & output = data.output;

  // Append the hide list from automatic material property output
  std::vector<std::string> material_hide;
  _app.getOutputWarehouse().buildMaterialOutputHideList(_name, material_hide);
  hide.insert(hide.end(), material_hide.begin(), material_hide.end());

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

Real
Output::time()
{
  if (_transient)
    return _time;
  else
    return _t_step;
}

Real
Output::timeOld()
{
  if (_transient)
    return _time_old;
  else
    return _t_step - 1;
}

Real
Output::dt()
{
  if (_transient)
    return _dt;
  else
    return 1;
}

Real
Output::dtOld()
{
  if (_transient)
    return _dt_old;
  else
    return 1;
}

int
Output::timeStep()
{
  return _t_step;
}
