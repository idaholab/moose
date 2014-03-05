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
#include "OutputBase.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "Postprocessor.h"
#include "Restartable.h"
#include "FileMesh.h"

template<>
InputParameters validParams<OutputBase>()
{

  /* NOTE:
   * The validParams from each output object is merged with the valdParams from CommonOutputAction. In order for the
   * common parameters to be applied correctly any parameter that is a common parameter (e.g., output_initial) MUST NOT
   * set a default value, this is because the default is extracted from the common parameters when the output object
   * is being created within AddOutputAction. */

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

  // Displaced Mesh options
  params.addParam<bool>("use_displaced", "Enable/disable the use of the displaced mesh for outputing");
  params.addParam<bool>("append_displaced", "Append '_displaced' to the output file base");
  params.addParamNamesToGroup("use_displaced, append_displaced", "Displaced");

  // Control for outputing elemental variables as nodal variables
  params.addParam<bool>("elemental_as_nodal", false, "Output elemental nonlinear variables as nodal");
  params.addParam<bool>("scalar_as_nodal", false, "Output scalar variables as nodal");

  // Output intervals and timing
  params.addParam<bool>("output_initial", "Request that the initial condition is output to the solution file");
  params.addParam<bool>("output_final", "Force the final timestep to be output, regardless of output interval");
  params.addParam<unsigned int>("interval", "The interval at which timesteps are output to the solution file");
  params.addParam<std::vector<Real> >("sync_times", "Times at which the output and solution is forced to occur");
  params.addParam<bool>("sync_only", false, "Only export results at sync times");

  // 'Timing' group
  params.addParamNamesToGroup("interval output_initial output_final sync_times sync_only", "Timing");

  // 'Variables' Group
  params.addParamNamesToGroup("hide show output_nonlinear_variables output_postprocessors output_scalar_variables output_elemental_variables output_nodal_variables scalar_as_nodal elemental_as_nodal", "Variables");

  // Register this class as base class
  params.registerBase("OutputBase");
  return params;
}

OutputBase::OutputBase(const std::string & name, InputParameters & parameters) :
    MooseObject(name, parameters),
    Restartable(name, parameters, "OutputBase"),
    _problem_ptr(getParam<FEProblem *>("_fe_problem")),
    _use_displaced(isParamValid("use_displaced") ? getParam<bool>("use_displaced") : false),
    _es_ptr(_use_displaced ? &_problem_ptr->getDisplacedProblem()->es() : &_problem_ptr->es()),
    _time(_problem_ptr->time()),
    _time_old(_problem_ptr->timeOld()),
    _t_step(_problem_ptr->timeStep()),
    _dt(_problem_ptr->dt()),
    _dt_old(_problem_ptr->dtOld()),
    _transient(_problem_ptr->isTransient()),
    _output_initial(isParamValid("output_initial") ? getParam<bool>("output_initial") : false),
    _output_final(isParamValid("output_final") ? getParam<bool>("output_final") : false),
    _output_input(getParam<bool>("output_input")),
    _elemental_as_nodal(getParam<bool>("elemental_as_nodal")),
    _scalar_as_nodal(getParam<bool>("scalar_as_nodal")),
    _system_information(getParam<bool>("output_system_information")),
    _mesh_changed(false),
    _sequence(_use_displaced),
    _num(declareRestartableData<int>("num", 0)),
    _interval(isParamValid("interval") ? getParam<unsigned int>("interval") : 1),
    _sync_times(isParamValid("sync_times") ?
                std::set<Real>(getParam<std::vector<Real> >("sync_times").begin(),
                               getParam<std::vector<Real> >("sync_times").end()) :
                std::set<Real>()),
    _sync_only(getParam<bool>("sync_only")),
    _allow_output(true),
    _force_output(false)
{
  // Initialize the available output
  initAvailable();

  // Seperate the hide/show list into components
  seperate(getParam<std::vector<VariableName> >("show"),
           getParam<std::vector<VariableName> >("hide"));

  // Intialize the show/hide/output lists for each of the types of output
  init(_nonlinear_elemental);
  init(_nonlinear_nodal);
  init(_scalar);
  init(_postprocessor);

  // If 'elemental_as_nodal = true' the elemental variable names must be appended to the
  // nodal variable names. Thus, when libMesh::EquationSystem::build_solution_vector is called
  // it will create the correct nodal variable from the elemental
  if (_elemental_as_nodal)
      _nonlinear_nodal.output.insert(_nonlinear_nodal.output.end(),
                                     _nonlinear_elemental.output.begin(),
                                     _nonlinear_elemental.output.end());

  // Similarly as above, if 'scalar_as_nodal = true' append the elemental variable list
  if (_scalar_as_nodal)
      _nonlinear_nodal.output.insert(_nonlinear_nodal.output.end(),
                                     _scalar.output.begin(),
                                     _scalar.output.end());

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
}

OutputBase::~OutputBase()
{
}

void
OutputBase::outputSetup()
{
}

void
OutputBase::initialSetup()
{
}

void
OutputBase::timestepSetup()
{
}

void
OutputBase::outputInitial()
{
  // Do nothing if output is not force or if output is disallowed
  if (!_force_output && !_allow_output)
    return;

  // Output the initial condition, if desired
  if (_force_output || _output_initial)
  {
    outputSetup();
    _mesh_changed = false;
    output();
    _num++;
  }

  // Output the input, if desired and it has not been output previously
  if ( _output_input )
  {
    // Produce warning if an input file does not exist
    // (parser/action system are not mandatory subsystems)
    if (_app.actionWarehouse().empty())
      mooseWarning("There is no input file to be output");

    // Call the input file output function
    outputInput();

    // Do not allow the input file to be written again
    _output_input = false;
  }

  // Set the force output flag to false
  _force_output = false;
}

void
OutputBase::outputStep()
{
  // Do nothing if output is not forced and is not allowed
  if (!_force_output && !_allow_output)
    return;

  // Only continue if output is not forced and the output is on an inveval
  if (!_force_output && !checkInterval())
    return;

  // If the mesh has changed or the sequence state is true, call the outputSetup() function
  if (_mesh_changed || _sequence || _num == 0)
  {
    // Execute the setup function
    outputSetup();

    // Reset the _mesh_changed flag
    _mesh_changed = false;
  }

  // Update the output number
  _num++;

  // Perform the output
  output();

  // Set the force output flag to false
  _force_output = false;
}

void
OutputBase::outputFinal()
{
  // Do nothing if output is not force or if output is disallowed
  if (!_force_output && !_allow_output)
    return;

  // Do nothing if the output is not forced and final output is not desired
  if (!_force_output && !_output_final)
    return;

  // If the mesh has changed or the sequence state is true, call the outputSetup() function
  if (_mesh_changed || _sequence)
    outputSetup();

  // Perform the output
  output();

  // Set the force output flag to false
  _force_output = false;
}

void
OutputBase::output()
{
  // Call the various output types, if data exists
  if (hasNodalVariableOutput())
    outputNodalVariables();

  if (hasElementalVariableOutput())
    outputElementalVariables();

  if (hasPostprocessorOutput())
    outputPostprocessors();

  if (hasScalarOutput())
    outputScalarVariables();
}

void
OutputBase::forceOutput()
{
  _force_output = true;
}

bool
OutputBase::hasOutput()
{
  // Test all the possible output formats, return true if any of them are true
  if (hasNodalVariableOutput() || hasElementalVariableOutput() ||
      hasScalarOutput() || hasPostprocessorOutput())
    return true;
  else
    return false;
}

void
OutputBase::outputInput()
{
  // Empty function
}

void
OutputBase::outputSystemInformation()
{
  // Empty function
}

bool
OutputBase::hasNodalVariableOutput()
{
  return !_nonlinear_nodal.output.empty();
}

const std::vector<std::string> &
OutputBase::getNodalVariableOutput()
{
  return _nonlinear_nodal.output;
}

bool
OutputBase::hasElementalVariableOutput()
{
  return !_nonlinear_elemental.output.empty();
}

const std::vector<std::string> &
OutputBase::getElementalVariableOutput()
{
  return _nonlinear_elemental.output;
}


bool
OutputBase::hasScalarOutput()
{
  return !_scalar.output.empty();
}

const std::vector<std::string> &
OutputBase::getScalarOutput()
{
  return _scalar.output;
}

bool
OutputBase::hasPostprocessorOutput()
{
  return !_postprocessor.output.empty();
}

const std::vector<std::string> &
OutputBase::getPostprocessorOutput()
{
  return _postprocessor.output;
}

void
OutputBase::meshChanged()
{
  _mesh_changed = true;
}

void
OutputBase::allowOutput(bool state)
{
  _allow_output = state;
}


void
OutputBase::sequence(bool state)
{
  _sequence = state;
}

bool
OutputBase::checkInterval()
{
  // The output flag to return
  bool output = false;

  // Return true if the current step on the current output interval
  if ( (_t_step % _interval) == 0)
    output = true;

  // Return false if 'sync_only' is set to true
  if (_sync_only)
    output = false;

  // If sync times are not skipped, return true if the current time is a sync_time
  if ( _sync_times.find(_time) != _sync_times.end() )
    output = true;

  // Return the output status
  return output;
}

void
OutputBase::initAvailable()
{
  /* This flag is set to true if any postprocessor has the 'outputs' parameter set, it is then used
     to produce an warning if postprocessor output is disabled*/
  bool has_limited_pps = false;

  // Get a reference to the storage of the postprocessors
  ExecStore<PostprocessorWarehouse> & warehouse = _problem_ptr->getPostprocessorWarehouse();

  // Possible execution type flags
  ExecFlagType types[] = { EXEC_TIMESTEP, EXEC_TIMESTEP_BEGIN, EXEC_INITIAL, EXEC_JACOBIAN, EXEC_RESIDUAL, EXEC_CUSTOM };

  // Loop through each of the execution flsgs
  for (unsigned int i = 0; i < LENGTHOF(types); i++)
  {
    // Loop through each of the postprocessors
    for (std::vector<Postprocessor *>::const_iterator postprocessor_it = warehouse(types[i])[0].all().begin();
         postprocessor_it != warehouse(types[i])[0].all().end();
         ++postprocessor_it)
    {
      // Store the name in the available postprocessors
      Postprocessor *pps = *postprocessor_it;
      _postprocessor.available.push_back(pps->PPName());

      // Extract the list of outputs
      std::set<OutputName> pps_outputs = pps->getOutputs();

      // Hide the postprocessor if 'none' is used within the 'outputs' parameter
      if (!pps_outputs.empty() && ( pps_outputs.find("none") != pps_outputs.end() || pps_outputs.find(_name) == pps_outputs.end() ) )
        _postprocessor.hide.push_back(pps->PPName());

      // Check that the output object allows postprocessor output
      if ( pps_outputs.find(_name) != pps_outputs.end() )
      {
        if (!isParamValid("output_postprocessors"))
          mooseWarning("Postprocessor '" << pps->PPName()
                       << "' has requested to be output by the '" << _name
                       << "' outputter, but postprocessor output is not support by this type of outputter.");
      }

      // Set the flag state for postprocessors that utilize 'outputs' parameter
      if (!pps_outputs.empty())
        has_limited_pps = true;
    }
  }

  // Produce the warning when 'outputs' is used, but postprocessor output is disable
  if (has_limited_pps && isParamValid("output_postprocessors") && getParam<bool>("output_postprocessors") == false)
    mooseWarning("A Postprocessor utilizes the 'outputs' parameter; however, postprocessor output is disable for the '" << _name << "' outputter.");

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
OutputBase::seperate(const std::vector<VariableName> & show, const std::vector<VariableName> & hide)
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
    else
      unknown.push_back(*it);
  }

  // Error if an unknown variable or postprocessor is found
  if (!unknown.empty())
  {
    std::ostringstream oss;
    oss << "Output(s) do not exist (must be variable, scalar, or postprocessor): " << (*unknown.begin());
    for (std::vector<std::string>::iterator it = unknown.begin()+1; it != unknown.end();  ++it)
      oss << ", " << *it;
    mooseError(oss.str());
  }
}

void
OutputBase::init(OutputData & data)
{
  // References to the vectors of variable names
  std::vector<std::string> & hide  = data.hide;
  std::vector<std::string> & show  = data.show;
  std::vector<std::string> & avail = data.available;
  std::vector<std::string> & output = data.output;

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

  // Only show is empty (show all execpt those hidden)
  else if (show.empty() && !hide.empty())
    std::set_difference(avail.begin(), avail.end(), hide.begin(), hide.end(), std::back_inserter(output));

  // Both hide and empty are present (show all those listed)
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
