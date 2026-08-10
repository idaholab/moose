//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckOutputFile.h"

#include "MooseApp.h"
#include "MooseError.h"
#include "OutputWarehouse.h"
#include "Output.h"
#include "FileOutput.h"

#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <sys/stat.h>

registerMooseObject("MooseTestApp", CheckOutputFile);

InputParameters
CheckOutputFile::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::string>("output_object",
                                       "The name of the Output object whose file will be checked.");
  return params;
}

CheckOutputFile::CheckOutputFile(const InputParameters & parameters)
  : GeneralUserObject(parameters), _output_object_name(getParam<std::string>("output_object"))
{
}

void
CheckOutputFile::execute()
{
}

void
CheckOutputFile::initialize()
{
}

void
CheckOutputFile::finalize()
{
}

void
CheckOutputFile::postExecute()
{
  FileOutput * file_output =
      dynamic_cast<FileOutput *>(_app.getOutputWarehouse().getOutput<Output>(_output_object_name));
  std::string local_path = file_output->filename();

  struct stat file_stat;
  if (stat(local_path.c_str(), &file_stat) != 0)
  {
    mooseError("File not found at '", local_path, "'.");
  }

  _console << "File found at '" << local_path << "'. \n" << std::flush;
}
