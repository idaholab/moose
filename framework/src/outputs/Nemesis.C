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
  InputParameters params = validParams<AdvancedOutput<OversampleOutput> >();
  params += AdvancedOutput<OversampleOutput>::enableOutputTypes("scalar postprocessor input");

  // Add description for the Nemesis class
  params.addClassDescription("Object for output data in the Nemesis format");

  // Return the InputParameters
  return params;
}

Nemesis::Nemesis(const std::string & name, InputParameters parameters) :
    AdvancedOutput<OversampleOutput>(name, parameters),
    _nemesis_io_ptr(NULL),
    _file_num(0),
    _nemesis_num(0),
    _nemesis_initialized(false)
{
}

Nemesis::~Nemesis()
{
  // Clean up the libMesh::NemesisII_IO object
  delete _nemesis_io_ptr;
}

void
Nemesis::initialSetup()
{
  // Call the base class method
  AdvancedOutput<OversampleOutput>::initialSetup();

  // Make certain that a Nemesis_IO object exists
  meshChanged();
}

void
Nemesis::meshChanged()
{
  // Maintain Oversample::meshChanged() functionality
  OversampleOutput::meshChanged();

  // Do not delete the Nemesis_IO object if it has not been used; also there is no need to setup
  // the object in this case, so just return
  if (_nemesis_io_ptr != NULL && !_nemesis_initialized)
    return;

  // Delete existing NemesisII_IO objects
  if (_nemesis_io_ptr != NULL)
    delete _nemesis_io_ptr;

  // Increment the file number
  _file_num++;

  // Reset the number of outputs for this file
  _nemesis_num = 1;

  // Create the new NemesisIO object
  _nemesis_io_ptr = new Nemesis_IO(_mesh_ptr->getMesh());
  _nemesis_initialized = false;

}

void
Nemesis::outputPostprocessors()
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
Nemesis::outputScalarVariables()
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
Nemesis::output(const ExecFlagType & type)
{
  if (!OversampleOutput::shouldOutput(type))
    return;

  // Clear the global variables (postprocessors and scalars)
  _global_names.clear();
  _global_values.clear();

  // Call the output methods
  AdvancedOutput<OversampleOutput>::output(type);

  // Write the data
  _nemesis_io_ptr->write_timestep(filename(), *_es_ptr, _nemesis_num, time() + _app.getGlobalTimeOffset());
  _nemesis_initialized = true;

  // Increment output call counter for the current file
  _nemesis_num++;

  // Write the global variables (populated by the output methods)
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
