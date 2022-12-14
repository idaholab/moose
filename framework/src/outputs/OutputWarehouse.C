//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "OutputWarehouse.h"
#include "Output.h"
#include "Console.h"
#include "FileOutput.h"
#include "Checkpoint.h"
#include "FEProblem.h"
#include "TableOutput.h"
#include "Exodus.h"

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

OutputWarehouse::OutputWarehouse(MooseApp & app)
  : PerfGraphInterface(app, "OutputWarehouse"),
    _app(app),
    _buffer_action_console_outputs(false),
    _common_params_ptr(NULL),
    _output_exec_flag(EXEC_CUSTOM),
    _force_output(false),
    _last_message_ended_in_newline(true),
    _last_buffer(NULL),
    _num_printed(0)
{
  // Set the reserved names
  _reserved.insert("none"); // allows 'none' to be used as a keyword in 'outputs' parameter
  _reserved.insert("all");  // allows 'all' to be used as a keyword in 'outputs' parameter
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
  TIME_SECTION("initialSetup", 5, "Setting Up Outputs");

  resetFileBase();

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
OutputWarehouse::customSetup(const ExecFlagType & exec_type)
{
  for (const auto & obj : _all_objects)
    obj->customSetup(exec_type);
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
OutputWarehouse::addOutput(std::shared_ptr<Output> const output)
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

  // Insert object sync times to the global set
  if (output->parameters().isParamValid("sync_times"))
  {
    std::vector<Real> sync_times = output->parameters().get<std::vector<Real>>("sync_times");
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
  if (_object_names.empty() && _app.actionWarehouse().hasActions("add_output"))
  {
    const auto & actions = _app.actionWarehouse().getActionListByName("add_output");
    for (const auto & act : actions)
      _object_names.insert(act->name());
  }
  return _object_names;
}

void
OutputWarehouse::addOutputFilename(const OutputName & obj_name, const OutFileBase & filename)
{
  _file_base_map[obj_name].insert(filename);
  for (const auto & it : _file_base_map)
    if (it.first != obj_name && it.second.find(filename) != it.second.end())
      mooseError("An output file with the name, ", filename, ", already exists.");
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
   * This is one of three locations where we explicitly flush the output buffers during a
   * simulation:
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

static std::mutex moose_console_mutex;

void
OutputWarehouse::mooseConsole()
{
  mooseConsole(_console_buffer);
}

void
OutputWarehouse::mooseConsole(std::ostringstream & buffer)
{
  std::lock_guard<std::mutex> lock(moose_console_mutex);

  std::string message = buffer.str();

  // If someone else is writing - then we may need a newline
  if (&buffer != _last_buffer && !_last_message_ended_in_newline)
    message = '\n' + message;

  // Loop through all Console Output objects and pass the current output buffer
  std::vector<Console *> objects = getOutputs<Console>();
  if (!objects.empty())
  {
    for (const auto & obj : objects)
      obj->mooseConsole(message);

    // Reset
    buffer.clear();
    buffer.str("");
  }
  else if (!_app.actionWarehouse().isTaskComplete("add_output"))
  {
    if (!_buffer_action_console_outputs)
    {
      // this will cause messages to console before its construction immediately flushed and
      // cleared.
      bool this_message_ends_in_newline = message.empty() ? true : message.back() == '\n';

      // If that last message ended in newline then this one may need
      // to start with indenting
      // Note that we only indent the first line if the last message ended in new line
      if (_app.multiAppLevel() > 0)
        MooseUtils::indentMessage(_app.name(), message, COLOR_CYAN, _last_message_ended_in_newline);

      Moose::out << message << std::flush;
      buffer.clear();
      buffer.str("");

      _last_message_ended_in_newline = this_message_ends_in_newline;
    }
  }

  _last_buffer = &buffer;

  _num_printed++;
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
OutputWarehouse::setCommonParameters(const InputParameters * params_ptr)
{
  _common_params_ptr = params_ptr;
}

const InputParameters *
OutputWarehouse::getCommonParameters() const
{
  return _common_params_ptr;
}

std::set<Real> &
OutputWarehouse::getSyncTimes()
{
  return _sync_times;
}

void
OutputWarehouse::addInterfaceHideVariables(const std::string & output_name,
                                           const std::set<std::string> & variable_names)
{
  _interface_map[output_name].insert(variable_names.begin(), variable_names.end());
}

void
OutputWarehouse::buildInterfaceHideVariables(const std::string & output_name,
                                             std::set<std::string> & hide)
{
  std::map<std::string, std::set<std::string>>::const_iterator it =
      _interface_map.find(output_name);
  if (it != _interface_map.end())
    hide = it->second;
}

void
OutputWarehouse::checkOutputs(const std::set<OutputName> & names)
{
  for (const auto & name : names)
    if (!isReservedName(name) && !hasOutput(name))
      mooseError("The output object '", name, "' is not a defined output object");
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

void
OutputWarehouse::reset()
{
  for (const auto & pair : _object_map)
  {
    auto * table = dynamic_cast<TableOutput *>(pair.second);
    if (table != NULL)
      table->clear();
    auto * exodus = dynamic_cast<Exodus *>(pair.second);
    if (exodus != NULL)
      exodus->clear();
  }
}

void
OutputWarehouse::resetFileBase()
{
  // Set the file base from the application to FileOutputs and add associated filenames
  for (const auto & obj : _all_objects)
  {
    FileOutput * file_output = dynamic_cast<FileOutput *>(obj);
    if (file_output)
    {
      const std::string file_base = obj->parameters().get<bool>("_built_by_moose")
                                        ? _app.getOutputFileBase()
                                        : (_app.getOutputFileBase(true) + "_" + obj->name());
      file_output->setFileBase(file_base);

      addOutputFilename(obj->name(), file_output->filename());
    }
  }
}
