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

#include "libmesh/utility.h"

InputParameters
FileOutput::validParams()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = PetscOutput::validParams();
  params.addClassDescription("Base class for all file-based output");
  params.addParam<std::string>(
      "file_base",
      "The desired solution output name without an extension. If not provided, MOOSE sets it "
      "with Outputs/file_base when available. Otherwise, MOOSE uses input file name and this "
      "object name for a master input or uses master file_base, the subapp name and this object "
      "name for a subapp input to set it.");
  params.addParam<bool>(
      "append_date", false, "When true the date and time are appended to the output filename.");
  params.addParam<std::string>("append_date_format",
                               "The format of the date/time to append, if not given UTC format "
                               "is used (see http://www.cplusplus.com/reference/ctime/strftime).");
  // Add the padding option and list it as 'Advanced'
  params.addParam<unsigned int>(
      "padding", 4, "The number of digits for the extension suffix (e.g., out.e-s002)");
  params.addParam<std::vector<std::string>>("output_if_base_contains",
                                            std::vector<std::string>(),
                                            "If this is supplied then output will only be done in "
                                            "the case that the output base contains one of these "
                                            "strings.  This is helpful in outputting only a subset "
                                            "of outputs when using MultiApps.");
  params.addParamNamesToGroup(
      "file_base append_date append_date_format padding output_if_base_contains",
      "File name customization");

  return params;
}

FileOutput::FileOutput(const InputParameters & parameters)
  : PetscOutput(parameters),
    _file_num(declareRecoverableData<unsigned int>("file_num", 0)),
    _padding(getParam<unsigned int>("padding")),
    _output_if_base_contains(getParam<std::vector<std::string>>("output_if_base_contains"))
{
  // If restarting reset the file number
  if (_app.isRestarting())
    _file_num = 0;

  if (isParamValid("file_base"))
  {
    // Check that we are the only process or not a subapp
    if (!_app.isUltimateMaster())
      if (_app.multiAppNumber() > 0)
        mooseError("The parameter 'file_base' may not be specified for a child app when the "
                   "MultiApp has multiple instances of the child app, since all instances would "
                   "use the same file base and thus write to the same file.");
    setFileBaseInternal(getParam<std::string>("file_base"));
  }
}

bool
FileOutput::shouldOutput()
{
  if (!checkFilename())
    return false;
  return Output::shouldOutput();
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
FileOutput::setFileBase(const std::string & file_base)
{
  if (!isParamValid("file_base"))
    setFileBaseInternal(file_base);
}

void
FileOutput::setFileBaseInternal(const std::string & file_base)
{
  _file_base = file_base;

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
  std::filesystem::path directory_base = _file_base;
  directory_base.remove_filename();
  if (directory_base.empty())
    directory_base = ".";
  // ensure relative path
  directory_base = std::filesystem::relative(std::filesystem::absolute(directory_base));

  std::filesystem::path possible_dir_to_make;
  for (auto it = directory_base.begin(); it != directory_base.end(); ++it)
  {
    possible_dir_to_make = possible_dir_to_make / *it;
    const auto dir_string = possible_dir_to_make.generic_string();
    if (_app.processor_id() == 0 && access(dir_string.c_str(), F_OK) == -1)
      // Directory does not exist. Create
      if (Utility::mkdir(dir_string.c_str()) == -1)
        mooseError("Could not create directory: " + dir_string + " for file base: " + _file_base);
  }
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
