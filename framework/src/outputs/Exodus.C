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

// Moose includes
#include "Exodus.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "ExodusFormatter.h"
#include "FileMesh.h"

template<>
InputParameters validParams<Exodus>()
{
  // Get the base class parameters
  InputParameters params = validParams<AdvancedOutput<OversampleOutput> >();
  params += AdvancedOutput<OversampleOutput>::enableOutputTypes("nodal elemental scalar postprocessor input");

  // Enable sequential file output (do not set default, the use_displace criteria relies on isParamValid, see Constructor)
  params.addParam<bool>("sequence", "Enable/disable sequential file output (enabled by default when 'use_displace = true', otherwise defaults to false");

  // Set the default padding to 3
  params.set<unsigned int>("padding") = 3;

  // Add description for the Exodus class
  params.addClassDescription("Object for output data in the Exodus II format");

  // Set outputting of the input to be on by default
  params.set<MultiMooseEnum>("output_input_on") = "initial";

  // Return the InputParameters
  return params;
}

Exodus::Exodus(const std::string & name, InputParameters parameters) :
    AdvancedOutput<OversampleOutput>(name, parameters),
    _exodus_initialized(false),
    _exodus_num(declareRestartableData<unsigned int>("exodus_num", 0)),
    _recovering(_app.isRecovering()),
    _exodus_mesh_changed(declareRestartableData<bool>("exodus_mesh_changed", true)),
    _sequence(isParamValid("sequence") ? getParam<bool>("sequence") : _use_displaced ? true : false)
{
}

Exodus::~Exodus()
{
}

void
Exodus::initialSetup()
{
  // Call base class setup method
  AdvancedOutput<OversampleOutput>::initialSetup();

  // The libMesh::ExodusII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the Exodus file.");

  // Test that some sort of variable output exists (case when all variables are disabled but input output is still enabled
  if (!hasNodalVariableOutput() && !hasElementalVariableOutput() && !hasPostprocessorOutput() && !hasScalarOutput())
    mooseError("The current settings results in only the input file and no variables being output to the Exodus file, this is not supported.");
}

void
Exodus::meshChanged()
{
  // Maintain Oversample::meshChanged() functionality
  OversampleOutput::meshChanged();

  // Indicate to the Exodus object that the mesh has changed
  _exodus_mesh_changed = true;
}

void
Exodus::sequence(bool state)
{
  _sequence = state;
}

void
Exodus::outputSetup()
{
  if (_exodus_io_ptr != NULL)
  {
    // Do nothing if the ExodusII_IO objects exists, but has not been initialized
    if (!_exodus_initialized && _exodus_io_ptr != NULL)
      return;

    // Do nothing if the output is using oversampling. In this case the mesh that is being output
    // has not been changed, so there is no need to create a new ExodusII_IO object
    if (_exodus_io_ptr != NULL && (_oversample || _change_position))
      return;

    // Do nothing if the mesh has not changed and sequential output is not desired
    if (!_exodus_mesh_changed && !_sequence)
      return;
  }

  // Create the ExodusII_IO object
  _exodus_io_ptr.reset(new ExodusII_IO(_es_ptr->get_mesh()));
  _exodus_initialized = false;

  // Increment file number and set appending status, append if all the following conditions are met:
  //   (2) If the application is recovering (not restarting)
  //   (2) The mesh has NOT changed
  //   (3) An existing Exodus file exists for appending (_exodus_num > 0)
  //   (4) Sequential output is NOT desired
  if (_recovering && !_exodus_mesh_changed && _exodus_num > 0 && !_sequence)
  {
    // Set the recovering flag to false so that this special case is not triggered again
    _recovering = false;

    // Set the append flag to true b/c on recover the file is being appended
    _exodus_io_ptr->append(true);
  }
  else
  {
    // Increment file counter
    if (_exodus_mesh_changed || _sequence)
      _file_num++;

    // Disable file appending and reset exodus file number count
    _exodus_io_ptr->append(false);
    _exodus_num = 1;
  }

  // Utilize the spatial dimensions
  if (_es_ptr->get_mesh().mesh_dimension() != 1)
    _exodus_io_ptr->use_mesh_dimension_instead_of_spatial_dimension(true);
}


void
Exodus::outputNodalVariables()
{
  // Set the output variable to the nodal variables
  std::vector<std::string> nodal(getNodalVariableOutput().begin(), getNodalVariableOutput().end());
  _exodus_io_ptr->set_output_variables(nodal);

  // Write the data via libMesh::ExodusII_IO
  _exodus_io_ptr->write_timestep(filename(), *_es_ptr, _exodus_num, time() + _app.getGlobalTimeOffset());
  _exodus_num++;

  // This satisfies the initialization of the ExodusII_IO object
  _exodus_initialized = true;
}

void
Exodus::outputElementalVariables()
{
  // Make sure the the file is ready for writing of elemental data
  if (!_exodus_initialized || !hasNodalVariableOutput())
    outputEmptyTimestep();

  // Write the elemental data
  std::vector<std::string> elemental(getElementalVariableOutput().begin(), getElementalVariableOutput().end());
  _exodus_io_ptr->set_output_variables(elemental);
  _exodus_io_ptr->write_element_data(*_es_ptr);
}

void
Exodus::outputPostprocessors()
{
  // List of desired postprocessor outputs
  const std::set<std::string> & pps = getPostprocessorOutput();

  // Append the postprocessor data to the global name value parameters; scalar outputs
  // also append these member variables
  for (std::set<std::string>::const_iterator it = pps.begin(); it != pps.end(); ++it)
  {
    _global_names.push_back(*it);
    _global_values.push_back(_problem_ptr->getPostprocessorValue(*it));
  }
}

void
Exodus::outputScalarVariables()
{
  // List of desired scalar outputs
  const std::set<std::string> & out = getScalarOutput();

  // Append the scalar to the global output lists
  for (std::set<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
  {
    VariableValue & variable = _problem_ptr->getScalarVariable(0, *it).sln();
    unsigned int n = variable.size();

    // If the scalar has a single component, output the name directly
    if (n == 1)
    {
      _global_names.push_back(*it);
      _global_values.push_back(variable[0]);
    }

    // If the scalar as many components add indices to the end of the name
    else
    {
      for (unsigned int i = 0; i < n; ++i)
      {
        std::ostringstream os;
        os << *it << "_" << i;
        _global_names.push_back(os.str());
        _global_values.push_back(variable[i]);
      }
    }
  }
}

void
Exodus::outputInput()
{
  // Format the input file
  ExodusFormatter syntax_formatter;
  syntax_formatter.printInputFile(_app.actionWarehouse());
  syntax_formatter.format();

  // Store the information
  _input_record = syntax_formatter.getInputFileRecord();
}


void
Exodus::output(const ExecFlagType & type)
{
  // Do nothing if there is nothing to output
  if (!hasOutput(type))
    return;

  // Start the performance log
  Moose::perf_log.push("output()", "Exodus");

  // Prepare the ExodusII_IO object
  outputSetup();

  // Adjust the position of the output
  if (_app.hasOutputPosition())
    _exodus_io_ptr->set_coordinate_offset(_app.getOutputPosition());

  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Call the individual output methods
  AdvancedOutput<OversampleOutput>::output(type);

  // Write the global variables (populated by the output methods)
  if (!_global_values.empty())
  {
    if (!_exodus_initialized)
      outputEmptyTimestep();
    _exodus_io_ptr->write_global_data(_global_values, _global_names);
  }

  // Write the input file record if it exists and the output file is initialized
  if (!_input_record.empty() && _exodus_initialized)
  {
     _exodus_io_ptr->write_information_records(_input_record);
    _input_record.clear();
  }

  // Reset the mesh changed flag
  _exodus_mesh_changed = false;

  // Stop the logging
  Moose::perf_log.pop("output()", "Exodus");
}

std::string
Exodus::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base + ".e";

  // Add the -s00x extension to the file
  if (_file_num > 1)
    output << "-s"
           << std::setw(_padding)
           << std::setprecision(0)
           << std::setfill('0')
           << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
Exodus::outputEmptyTimestep()
{
  // Write a timestep with no variables
  _exodus_io_ptr->set_output_variables(std::vector<std::string>());
  _exodus_io_ptr->write_timestep(filename(), *_es_ptr, _exodus_num, time() + _app.getGlobalTimeOffset());
  _exodus_num++;
  _exodus_initialized = true;
}
