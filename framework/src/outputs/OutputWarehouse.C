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
#include "OutputWarehouse.h"
#include "Output.h"
#include "Console.h"
#include "FileOutput.h"
#include "Checkpoint.h"
#include "FEProblem.h"

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

OutputWarehouse::OutputWarehouse(MooseApp & app) :
    _app(app),
    _buffer_action_console_outputs(false),
    _output_exec_flag(EXEC_CUSTOM),
    _force_output(false),
    _logging_requested(false)
{
  // Set the reserved names
  _reserved.insert("none");                  // allows 'none' to be used as a keyword in 'outputs' parameter
  _reserved.insert("all");                   // allows 'all' to be used as a keyword in 'outputs' parameter
}

OutputWarehouse::~OutputWarehouse()
{
  // If the output buffer is not empty, it needs to be written
  if (_console_buffer.str().length())
    mooseConsole();
}

void
OutputWarehouse::initialSetup()
{
  for (const auto & obj : _all_objects)
    obj->initialSetup();
}

void
OutputWarehouse::timestepSetup()
{
  for (const auto & obj : _all_objects)
    obj->timestepSetup();
}

void
OutputWarehouse::solveSetup()
{
  for (const auto & obj : _all_objects)
    obj->solveSetup();
}

void
OutputWarehouse::jacobianSetup()
{
  for (const auto & obj : _all_objects)
    obj->jacobianSetup();
}

void
OutputWarehouse::residualSetup()
{
  for (const auto & obj : _all_objects)
    obj->residualSetup();
}

void
OutputWarehouse::subdomainSetup()
{
  for (const auto & obj : _all_objects)
    obj->subdomainSetup();
}

void
OutputWarehouse::addOutput(std::shared_ptr<Output> & output)
{
  _all_ptrs.push_back(output);

  // Add the object to the warehouse storage, Checkpoint placed at end so they are called last
  Checkpoint * cp = dynamic_cast<Checkpoint *>(output.get());
  if (cp != NULL)
    _all_objects.push_back(output.get());
  else
    _all_objects.insert(_all_objects.begin(), output.get());

  // Store the name and pointer
  _object_map[output->name()] = output.get();
  _object_names.insert(output->name());

  // If the output object is a FileOutput then store the output filename
  FileOutput * ptr = dynamic_cast<FileOutput *>(output.get());
  if (ptr != NULL)
    addOutputFilename(ptr->filename());

  // Insert object sync times to the global set
  if (output->parameters().isParamValid("sync_times"))
  {
    std::vector<Real> sync_times = output->parameters().get<std::vector<Real> >("sync_times");
    _sync_times.insert(sync_times.begin(), sync_times.end());
  }
}

bool
OutputWarehouse::hasOutput(const std::string & name) const
{
  return _object_map.find(name) != _object_map.end();
}

const std::set<OutputName> &
OutputWarehouse::getOutputNames()
{
  if (_object_names.empty())
  {
    const auto & actions = _app.actionWarehouse().getActionListByName("add_output");
    for (const auto & act : actions)
      _object_names.insert(act->name());
  }
  return _object_names;
}

void
OutputWarehouse::addOutputFilename(const OutFileBase & filename)
{
  if (_file_base_set.find(filename) != _file_base_set.end())
    mooseError2("An output file with the name, ", filename, ", already exists.");
  _file_base_set.insert(filename);
}

void
OutputWarehouse::outputStep(ExecFlagType type)
{
  if (_force_output)
    type = EXEC_FORCED;

  for (const auto & obj : _all_objects)
    if (obj->enabled())
      obj->outputStep(type);

  /**
   * This is one of three locations where we explicitly flush the output buffers during a simulation:
   * PetscOutput::petscNonlinearOutput()
   * PetscOutput::petscLinearOutput()
   * OutputWarehouse::outputStep()
   *
   * All other Console output _should_ be using newlines to avoid covering buffer errors
   * and to avoid excessive I/O
   */
  flushConsoleBuffer();

  // Reset force output flag
  _force_output = false;
}

void
OutputWarehouse::meshChanged()
{
  for (const auto & obj : _all_objects)
    obj->meshChanged();
}

void
OutputWarehouse::mooseConsole()
{
  // Loop through all Console Output objects and pass the current output buffer
  std::vector<Console *> objects = getOutputs<Console>();
  if (!objects.empty())
  {
    for (const auto & obj : objects)
      obj->mooseConsole(_console_buffer.str());

    // Reset
    _console_buffer.clear();
    _console_buffer.str("");
  }
  else
  {
    if (!_buffer_action_console_outputs)
    {
      // this will cause messages to console before its construction immediately flushed and cleared.
      std::string message = _console_buffer.str();
      if (_app.multiAppLevel() > 0)
        MooseUtils::indentMessage(_app.name(), message);
      Moose::out << message;
      _console_buffer.clear();
      _console_buffer.str("");
    }
  }
}

void
OutputWarehouse::flushConsoleBuffer()
{
  if (!_console_buffer.str().empty())
    mooseConsole();
}

void
OutputWarehouse::setFileNumbers(std::map<std::string, unsigned int> input, unsigned int offset)
{
  for (const auto & obj : _all_objects)
  {
    FileOutput * ptr = dynamic_cast<FileOutput *>(obj);
    if (ptr != NULL)
    {
      std::map<std::string, unsigned int>::const_iterator it = input.find(ptr->name());
      if (it != input.end())
      {
        int value = it->second + offset;
        if (value < 0)
          ptr->setFileNumber(0);
        else
          ptr->setFileNumber(it->second + offset);
      }
    }
  }
}

std::map<std::string, unsigned int>
OutputWarehouse::getFileNumbers()
{

  std::map<std::string, unsigned int> output;
  for (const auto & obj : _all_objects)
  {
    FileOutput * ptr = dynamic_cast<FileOutput *>(obj);
    if (ptr != NULL)
      output[ptr->name()] = ptr->getFileNumber();
  }
  return output;
}

void
OutputWarehouse::setCommonParameters(InputParameters * params_ptr)
{
  _common_params_ptr = params_ptr;
}

InputParameters *
OutputWarehouse::getCommonParameters()
{
  return _common_params_ptr;
}

std::set<Real> &
OutputWarehouse::getSyncTimes()
{
  return _sync_times;
}

void
OutputWarehouse::addInterfaceHideVariables(const std::string & output_name, const std::set<std::string> & variable_names)
{
  _interface_map[output_name].insert(variable_names.begin(), variable_names.end());
}

void
OutputWarehouse::buildInterfaceHideVariables(const std::string & output_name, std::set<std::string> & hide)
{
  std::map<std::string, std::set<std::string> >::const_iterator it = _interface_map.find(output_name);
  if (it != _interface_map.end())
    hide = it->second;
}

void
OutputWarehouse::checkOutputs(const std::set<OutputName> & names)
{
  for (const auto & name : names)
    if (!isReservedName(name) && !hasOutput(name))
      mooseError2("The output object '", name, "' is not a defined output object");
}


const std::set<std::string> &
OutputWarehouse::getReservedNames() const
{
  return _reserved;
}

bool
OutputWarehouse::isReservedName(const std::string & name)
{
  return _reserved.find(name) != _reserved.end();
}

void
OutputWarehouse::setOutputExecutionType(ExecFlagType type)
{
  _output_exec_flag = type;
}

void
OutputWarehouse::allowOutput(bool state)
{
  for (const auto & obj : _all_objects)
    obj->allowOutput(state);
}

void
OutputWarehouse::forceOutput()
{
  _force_output = true;
}
