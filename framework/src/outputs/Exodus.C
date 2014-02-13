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

  InputParameters params = validParams<OversampleOutputter>();
  params += validParams<FileOutputInterface>();

  params.addParam<unsigned int>("padding", 3, "The number of digits for the -s extension (e.g., out.e-s002)");
  params.addParamNamesToGroup("padding", "Advanced");

  // Add description for the Exodus class
  params.addClassDescription("Object for output data in the Exodus II format");

  // Return the InputParameters
  return params;
}

Exodus::Exodus(const std::string & name, InputParameters parameters) :
    OversampleOutputter(name, parameters),
    FileOutputInterface(name, parameters),
    _exodus_io_ptr(NULL),
    _file_num(0),
    _initialized(false),
    _padding(getParam<unsigned int>("padding")),
    _exodus_num(0)
{
}

Exodus::~Exodus()
{
  // Clean up the libMesh::ExodusII_IO object
  delete _exodus_io_ptr;
}

void
Exodus::outputSetup()
{
  // The libMesh::ExodusII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the Exodus file.");

  // Delete existing ExodusII_IO objects
  if (_exodus_io_ptr != NULL)
    delete _exodus_io_ptr;

  // Increment the file number
  _file_num++;

  // Reset the number of outputs for this file
  _exodus_num = 1;

  // Create the new ExodusII_IO object
  _exodus_io_ptr = new ExodusII_IO(_es_ptr->get_mesh());
  _exodus_io_ptr->append(false);

  // Utilize the spatial dimensions
  if (_es_ptr->get_mesh().mesh_dimension() != 1)
    _exodus_io_ptr->use_mesh_dimension_instead_of_spatial_dimension(true);

}

void
Exodus::outputNodalVariables()
{
  // Set the output variable to the nodal variables
  _exodus_io_ptr->set_output_variables(getNodalVariableOutput());

  // Write the data via libMesh::ExodusII_IO
  _exodus_io_ptr->write_timestep(filename(), *_es_ptr, _exodus_num, _time + _app.getGlobalTimeOffset());

  // This satisfies the initialization of the ExodusII_IO object
  _initialized = true;
}

void
Exodus::outputElementalVariables()
{
  // Make sure the the file is ready for writting of elemental data
  if (!_initialized)
    outputEmptyTimestep();

  // Write the elemental data
  _exodus_io_ptr->set_output_variables(getElementalVariableOutput());
  _exodus_io_ptr->write_element_data(*_es_ptr);
}

void
Exodus::outputPostprocessors()
{
  // List of desired postprocessor outputs
  const std::vector<std::string> & pps = getPostprocessorOutput();

  // Append the postprocessor data to the global name value parameters; scalar outputs
  // also append these member variables
  for (std::vector<std::string>::const_iterator it = pps.begin(); it != pps.end(); ++it)
  {
    _global_names.push_back(*it);
    _global_values.push_back(_problem_ptr->getPostprocessorValue(*it));
  }

}

void
Exodus::outputScalarVariables()
{
  // List of desired scalar outputs
  const std::vector<std::string> & out = getScalarOutput();

  // Append the scalar to the global output lists
  for (std::vector<std::string>::const_iterator it = out.begin(); it != out.end(); ++it)
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

  // Write the input to the output
  _exodus_io_ptr->write_information_records(syntax_formatter.getInputFileRecord());
}


void
Exodus::output()
{

  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Call the output methods
  OversampleOutputter::output();

  // Increment output call counter, which is reset by outputSetup and outputSetup
  // is automatically called by OversampleBase::output()
  _exodus_num++;

  // Write the global variabls (populated by the output methods)
  if (!_global_values.empty())
  {
    if (!_initialized)
      outputEmptyTimestep();
    _exodus_io_ptr->write_global_data(_global_values, _global_names);
  }
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
  _exodus_io_ptr->write_timestep(filename(), *_es_ptr, _exodus_num, _time + _app.getGlobalTimeOffset());
  _initialized = true;
}
