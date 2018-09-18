//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// C POSIX includes
#include <sys/stat.h>

// MOOSE includes
#include "FileOutput.h"
#include "MooseApp.h"
#include "FEProblem.h"

#include <unistd.h>
#include <ctime>

template <>
InputParameters
validParams<FileOutput>()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = validParams<PetscOutput>();
  params.addParam<std::string>("file_base",
                               "The desired solution output name without an extension");
  params.addParam<bool>(
      "append_date", false, "When true the date and time are appended to the output filename.");
  params.addParam<std::string>("append_date_format",
                               "The format of the date/time to append, if not given UTC format "
                               "used (see http://www.cplusplus.com/reference/ctime/strftime).");
  // Add the padding option and list it as 'Advanced'
  params.addParam<unsigned int>(
      "padding", 4, "The number of for extension suffix (e.g., out.e-s002)");
  params.addParam<std::vector<std::string>>("output_if_base_contains",
                                            std::vector<std::string>(),
                                            "If this is supplied then output will only be done in "
                                            "the case that the output base contains one of these "
                                            "strings.  This is helpful in outputting only a subset "
                                            "of outputs when using MultiApps.");
  params.addParamNamesToGroup("padding output_if_base_contains", "Advanced");

  return params;
}

FileOutput::FileOutput(const InputParameters & parameters)
  : PetscOutput(parameters),
    _file_num(declareRecoverableData<unsigned int>("file_num", 0)),
    _padding(getParam<unsigned int>("padding")),
    _output_if_base_contains(parameters.get<std::vector<std::string>>("output_if_base_contains"))
{
  // If restarting reset the file number
  if (_app.isRestarting())
    _file_num = 0;

  // Set the file base
  if (isParamValid("file_base"))
  {
    _file_base = getParam<std::string>("file_base");
    if (!_file_base.empty() && _file_base[0] == '/')
      mooseError("absolute paths not allowed in output 'file_base' param");
  }
  else if (getParam<bool>("_built_by_moose"))
    _file_base = getOutputFileBase(_app);
  else
    _file_base = getOutputFileBase(_app, "_" + name());

  // Append the date/time
  if (getParam<bool>("append_date"))
  {
    std::string format;
    if (isParamValid("append_date_format"))
      format = getParam<std::string>("append_date_format");
    else
      format = "%Y-%m-%dT%T%z";

    // Get the current time
    std::time_t now;
    ::time(&now); // need :: to avoid confusion with time() method of Output class

    // Format the time
    char buffer[80];
    strftime(buffer, 80, format.c_str(), localtime(&now));
    _file_base += "_";
    _file_base += buffer;
  }

  // Check the file directory of file_base and create if needed
  std::string base = "./" + _file_base;
  base = base.substr(0, base.find_last_of('/'));

  if (_app.processor_id() == 0 && access(base.c_str(), W_OK) == -1)
  {
    // Directory does not exist. Loop through incremental directories and create as needed.
    std::vector<std::string> path_names;
    MooseUtils::tokenize(base, path_names);
    std::string inc_path = path_names[0];
    for (unsigned int i = 1; i < path_names.size(); ++i)
    {
      inc_path += '/' + path_names[i];
      if (access(inc_path.c_str(), W_OK) == -1)
        if (mkdir(inc_path.c_str(), S_IRWXU | S_IRGRP) == -1)
          mooseError("Could not create directory: " + inc_path + " for file base: " + _file_base);
    }
  }
}

std::string
FileOutput::getOutputFileBase(const MooseApp & app, std::string suffix)
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
  size_t start = 0;
  if (input_filename.find_last_of('/') != std::string::npos)
    start = input_filename.find_last_of('/') + 1;
  return input_filename.substr(start, pos - start) + suffix;
}

bool
FileOutput::shouldOutput(const ExecFlagType & type)
{
  if (!checkFilename())
    return false;
  return Output::shouldOutput(type);
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
  for (const auto & search_string : _output_if_base_contains)
  {
    // Search for the string in the file base, if found set the output to true and break the loop
    if (_file_base.find(search_string) != std::string::npos)
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
