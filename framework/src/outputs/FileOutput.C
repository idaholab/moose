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

// MOOSE includes
#include "FileOutput.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include <unistd.h>

template<>
InputParameters validParams<FileOutput>()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = validParams<PetscOutput>();
  params.addParam<std::string>("file_base", "The desired solution output name without an extension");

  // Add the padding option and list it as 'Advanced'
  params.addParam<unsigned int>("padding", 4, "The number of for extension suffix (e.g., out.e-s002)");
  params.addParam<std::vector<std::string> >("output_if_base_contains", std::vector<std::string>(), "If this is supplied then output will only be done in the case that the output base contains one of these strings.  This is helpful in outputting only a subset of outputs when using MultiApps.");
  params.addParamNamesToGroup("padding output_if_base_contains", "Advanced");

  // **** DEPRECATED AND REMOVED PARAMETERS ****
  params.addDeprecatedParam<bool>("append_displaced", false, "Append '_displaced' to the output file base",
                                  "This parameter is no longer operational, to append '_displaced' utilize the output block name or 'file_base'");

  return params;
}

FileOutput::FileOutput(const std::string & name, InputParameters & parameters) :
    PetscOutput(name, parameters),
    _file_num(declareRecoverableData<unsigned int>("file_num", 0)),
    _padding(getParam<unsigned int>("padding")),
    _output_if_base_contains(parameters.get<std::vector<std::string> >("output_if_base_contains"))
{
  // If restarting reset the file number
  if (_app.isRestarting())
    _file_num = 0;

  // Set the file base
  if (isParamValid("file_base"))
    _file_base = getParam<std::string>("file_base");
  else if (getParam<bool>("_built_by_moose"))
    _file_base = getOutputFileBase(_app);
  else
    _file_base = getOutputFileBase(_app, "_" + name);

  // Check the file directory of file_base
  std::string base = "./" + _file_base;
  base = base.substr(0, base.find_last_of('/'));
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + _file_base);

  // ** DEPRECATED SUPPORT **
  if (getParam<bool>("append_displaced"))
    _file_base += "_displaced";

}

FileOutput::~FileOutput()
{
}

std::string
FileOutput::getOutputFileBase(MooseApp & app, std::string suffix)
{
  // If the App has an outputfile, then use it (MultiApp scenario)
  if (!app.getOutputFileBase().empty())
    return app.getOutputFileBase();

  // If the output base is not set it must be determined from the input file
  /* This will only return a non-empty string if the setInputFileName() was called, which is
   * generally not the case. One exception is when CoupledExecutioner is used */
  std::string input_filename = app.getInputFileName();
  if (input_filename.empty())
    input_filename = app.getFileName();

  // Assert that the filename is not empty
  mooseAssert(!input_filename.empty(), "Input Filename is NULL");

  // Determine location of "." in extension, assert if it is not found
  size_t pos = input_filename.find_last_of('.');
  mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");

  // Append the "_out" to the name and return it
  return input_filename.substr(0, pos) + suffix;
}

bool
FileOutput::shouldOutput(const ExecFlagType & type)
{
  if (checkFilename())
    return PetscOutput::shouldOutput(type);
  return false;
}

bool
FileOutput::checkFilename()
{
  // Return true if 'output_if_base_contains' is not utilized
  if (_output_if_base_contains.empty())
    return true;

  // Assumed output is false
  bool output = false;

  // Loop through each string in the list
  for (std::vector<std::string>::const_iterator it = _output_if_base_contains.begin(); it != _output_if_base_contains.end(); ++it)
  {
    // Search for the string in the file base, if found set the output to true and break the loop
    if (_file_base.find(*it) != std::string::npos)
    {
      output = true;
      break;
    }
  }

  // Return the value
  return output;
}

std::string
FileOutput::filename()
{
  return _file_base;
}

void
FileOutput::setFileNumber(unsigned int num)
{
  _file_num = num;
}

unsigned int
FileOutput::getFileNumber()
{
  return _file_num;
}
