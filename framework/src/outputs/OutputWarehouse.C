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
#include "OutputBase.h"
#include "Console.h"
#include "FileOutputter.h"
#include "Checkpoint.h"
#include "FEProblem.h"

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tinydir.h"
#include "pcrecpp.h"


OutputWarehouse::OutputWarehouse() :
    _has_screen_console(false)
{
}

OutputWarehouse::~OutputWarehouse()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    delete *it;
}

void
OutputWarehouse::initialSetup()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->initialSetup();
}


void
OutputWarehouse::init()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->init();
}

void
OutputWarehouse::timestepSetup()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
  {
    (*it)->timestepSetupInternal();
    (*it)->timestepSetup();
  }
}

void
OutputWarehouse::addOutput(OutputBase * output)
{
  // Add the object to the warehouse storage, Checkpoint placed at end so they are called last
  Checkpoint * cp = dynamic_cast<Checkpoint *>(output);
  if (cp != NULL)
    _object_ptrs.push_back(output);
  else
    _object_ptrs.insert(_object_ptrs.begin(), output);

  // Store the names
  _object_names.insert(output->name());

  // Store the name and pointer in map
  _object_map[output->name()] = output;

  // If the output object is a FileOutputBase then store the output filename
  FileOutputter * ptr = dynamic_cast<FileOutputter *>(output);
  if (ptr != NULL)
    addOutputFilename(ptr->filename());

  // Insert object sync times to the global set
  if (output->parameters().isParamValid("sync_times"))
  {
    std::vector<Real> sync_times = output->parameters().get<std::vector<Real> >("sync_times");
    _sync_times.insert(sync_times.begin(), sync_times.end());
  }

  // Warning if multiple Console objects are added with 'output_screen=true' in the input file
  Console * c_ptr = dynamic_cast<Console *>(output);
  if (c_ptr != NULL)
  {
    bool screen = c_ptr->getParam<bool>("output_screen");

    if (screen && _has_screen_console)
      mooseWarning("Multiple Console output objects are writing to the screen, this will likely cause duplicate messages printed.");
    else
      _has_screen_console = true;
  }
}

bool
OutputWarehouse::hasOutput(std::string name)
{
  return _object_names.find(name) != _object_names.end();
}

const std::vector<OutputBase *> &
OutputWarehouse::getOutputs() const
{
  return _object_ptrs;
}

void
OutputWarehouse::addOutputFilename(OutFileBase filename)
{
  if (_filenames.find(filename) != _filenames.end())
    mooseError("An output file with the name, " << filename << ", already exists.");

  _filenames.insert(filename);
}

void
OutputWarehouse::outputInitial()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->outputInitial();
}

void
OutputWarehouse::outputFailedStep()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->outputFailedStep();
}

void
OutputWarehouse::outputStep()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->outputStep();
}

void
OutputWarehouse::outputFinal()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->outputFinal();
}

void
OutputWarehouse::meshChanged()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->meshChanged();
}

void
OutputWarehouse::allowOutput(bool state)
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->allowOutput(state);
}

void
OutputWarehouse::forceOutput()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->forceOutput();
}

void
OutputWarehouse::setFileNumbers(std::map<std::string, unsigned int> input, unsigned int offset)
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
  {
    FileOutputter * ptr = dynamic_cast<FileOutputter *>(*it);
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
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
  {
    FileOutputter * ptr = dynamic_cast<FileOutputter *>(*it);
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

InputParameters &
OutputWarehouse::getCommonParameters()
{
  if (_common_params_ptr == NULL)
    mooseError("No common input parameters are stored");

  return *_common_params_ptr;
}

std::set<Real> &
OutputWarehouse::getSyncTimes()
{
  return _sync_times;
}
