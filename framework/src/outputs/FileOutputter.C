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
#include "FileOutputter.h"
#include "MooseApp.h"

template<>
InputParameters validParams<FileOutputter>()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = validParams<OutputBase>();
  params.addParam<std::string>("file_base", "The desired solution output name without an extension (Defaults appends '_out' to the input file name)");
  params.addParam<bool>("append_displaced", false, "Append '_displaced' to the output file base");

  // Add the padding option and list it as 'Advanced'
  params.addParam<unsigned int>("padding", 4, "The number of for extension suffix (e.g., out.e-s002)");
  params.addParam<std::vector<std::string> >("output_if_base_contains", "If this is supplied then output will only be done in the case that the output base contains one of these strings.  This is helpful in outputing only a subset of outputs when using MultiApps.");
  params.addParamNamesToGroup("padding output_if_base_contains", "Advanced");

  return params;
}

FileOutputter::FileOutputter(const std::string & name, InputParameters & parameters) :
    OutputBase(name, parameters),
    _file_base(getParam<std::string>("file_base")),
    _file_num(0),
    _padding(getParam<unsigned int>("padding")),
    _output_file(true)
{

  // Set the file base, if it has not been set already
  if (!isParamValid("file_base"))
    _file_base = getOutputFileBase();

  // Check the file directory of file_base
  std::string base = "./" + _file_base;
  base = base.substr(0, base.find_last_of('/'));
  if (access(base.c_str(), W_OK) == -1)
    mooseError("Can not write to directory: " + base + " for file base: " + _file_base);

  // Append the 'displaced' name, if desired and displaced mesh is being used
  if (isParamValid("use_displaced") && getParam<bool>("use_displaced") &&
      isParamValid("append_displaced") && getParam<bool>("append_displaced"))
    _file_base = _file_base + "_displaced";

  // Update the file_base check
  _output_file = checkFilename();
}

FileOutputter::~FileOutputter()
{
}

void
FileOutputter::outputInitial()
{
  // Perform filename check
  if (!_output_file)
    return;

  // Call the initial output method
  OutputBase::outputInitial();
}

void
FileOutputter::outputStep()
{

  // Perform filename check
  if (!_output_file)
    return;

  // Call the step output method
  OutputBase::outputStep();
}

void
FileOutputter::outputFinal()
{
  // Perform filename check
  if (!_output_file)
    return;

  // Call the final output methods
  OutputBase::outputFinal();
}

std::string
FileOutputter::getOutputFileBase()
{
  // If the App has an outputfile, then use it (MultiApp scenario)
  if (!_app.getOutputFileBase().empty())
    return _app.getOutputFileBase();

  // If the output base is not set it must be determined from the input file
  /* This will only return a non-empty string if the setInputFileName() was called, which is
   * generally not the case. One exception is when CoupledExecutioner is used */
  std::string input_filename = _app.getInputFileName();
  if (input_filename.empty())
    input_filename = _app.getFileName();

  // Asset that the filename is not empty
  mooseAssert(!input_filename.empty(), "Input Filename is NULL");

  // Determine location of "." in extension, assert if it is not found
  size_t pos = input_filename.find_last_of('.');
  mooseAssert(pos != std::string::npos, "Unable to determine suffix of input file name");

  // Append the "_out" to the name and return it
  return input_filename.substr(0, pos) + "_out";
}

bool
FileOutputter::checkFilename()
{
  // Return true if 'output_if_base_contains' is not utilized
  if (!isParamValid("output_if_base_contains"))
    return true;

  // Get the file list
  std::vector<std::string> contain = getParam<std::vector<std::string> >("output_if_base_contains");

  // Return true if it is empty
  if (contain.empty())
    return true;

  // Assumed output is false
  bool output = false;

  // Loop through each string in the list
  for (std::vector<std::string>::const_iterator it = contain.begin(); it != contain.end(); ++it)
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


void
FileOutputter::setFileNumber(unsigned int num)
{
  _file_num = num;
}

unsigned int
FileOutputter::getFileNumber()
{
  return _file_num;
}
