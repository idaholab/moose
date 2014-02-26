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
#include "Nemesis.h"
#include "MooseApp.h"
#include "FEProblem.h"

template<>
InputParameters validParams<Nemesis>()
{
  // Get the base class parameters
  InputParameters params = validParams<OversampleOutputter>();
  params += validParams<FileOutputInterface>();

  // Add description for the Nemesis class
  params.addClassDescription("Object for output data in the Nemesis format");

  // Return the InputParameters
  return params;
}

Nemesis::Nemesis(const std::string & name, InputParameters parameters) :
    OversampleOutputter(name, parameters),
    FileOutputInterface(name, parameters),
    _nemesis_io_ptr(NULL),
    _file_num(0),
    _nemesis_num(0)
{
}

Nemesis::~Nemesis()
{
  // Clean up the libMesh::NemesisII_IO object
  delete _nemesis_io_ptr;
}

void
Nemesis::outputSetup()
{
  // The libMesh::NemesisII_IO will fail when it is closed if the object is created but
  // nothing is written to the file. This checks that at least something will be written.
  if (!hasOutput())
    mooseError("The current settings result in nothing being output to the Nemesis file.");

  // Delete existing NemesisII_IO objects
  if (_nemesis_io_ptr != NULL)
    delete _nemesis_io_ptr;

  // Increment the file number
  _file_num++;

  // Reset the number of outputs for this file
  _nemesis_num = 1;

  // Create the new NemesisIO object
  _nemesis_io_ptr = new Nemesis_IO(_mesh_ptr->getMesh());
}

void
Nemesis::outputNodalVariables()
{
  // Empty for Nemesis output
}

void
Nemesis::outputElementalVariables()
{
  // Empty for Nemesis output
}

void
Nemesis::outputPostprocessors()
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
Nemesis::outputScalarVariables()
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
Nemesis::output()
{
  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Write the data
  _nemesis_io_ptr->write_timestep(filename(), *_es_ptr, _nemesis_num, _time + _app.getGlobalTimeOffset());

  // Increment output call counter for the current file
  _nemesis_num++;

  // Write the global variabls (populated by the output methods)
  if (!_global_values.empty())
    _nemesis_io_ptr->write_global_data(_global_values, _global_names);
}

std::string
Nemesis::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base << ".e" ;

  // Add the _000x extension to the file
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
