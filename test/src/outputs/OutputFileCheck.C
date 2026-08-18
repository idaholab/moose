//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutputFileCheck.h"

#include "MooseApp.h"

registerMooseObject("MooseTestApp", OutputFileCheck);

InputParameters
OutputFileCheck::validParams()
{
  InputParameters params = Output::validParams();

  params.addClassDescription("This object depend on other Output objects and check if the "
                             "corresponding output file was written.");

  params.addRequiredParam<std::string>("output_object",
                                       "The name of the Output object whose file will be checked.");

  return params;
}

OutputFileCheck::OutputFileCheck(const InputParameters & parameters)
  : Output(parameters), _output_object_name(getParam<std::string>("output_object"))
{
  // Declare dependency on the target output object.
  // OutputWarehouse::sortOutputs() reads this set to
  // ensure the target executes before this object.
  _depend_obj.insert(_output_object_name);

  // This object supplies itself
  _supplied_obj.insert(name());
}

void
OutputFileCheck::initialSetup()
{
  // If the user did not explicitly set execute_on,
  // inherit it from the target output object so the
  // two always fire on the same events.
  if (!isParamSetByUser("execute_on"))
  {
    OutputWarehouse & warehouse = _app.getOutputWarehouse();
    _execute_on =
        warehouse.getOutput<Output>(_output_object_name)->getParam<ExecFlagEnum>("execute_on");
  }
}

void
OutputFileCheck::output()
{
  OutputWarehouse & warehouse = _app.getOutputWarehouse();
  FileOutput * _file_output =
      dynamic_cast<FileOutput *>(warehouse.getOutput<Output>(_output_object_name));

  std::string local_path = _file_output->filename();

  struct stat file_stat;
  if (stat(local_path.c_str(), &file_stat) != 0)
  {
    mooseError("OutputFileCheck: File not found at '", local_path, "'.");
  }

  _console << "File found at '" << local_path << "'. \n" << std::flush;
}
