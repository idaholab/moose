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

#include "OutputWarehouse.h"
#include "OutputBase.h"
#include "FileOutputBase.h"
#include "Console.h"

OutputWarehouse::OutputWarehouse() :
    _has_console(false)
{
}

OutputWarehouse::~OutputWarehouse()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    delete *it;
}

void
OutputWarehouse::addOutput(OutputBase * output)
{

  // Add the object to the warehouse storage
  _object_ptrs.push_back(output);

  // Store the name
  _output_names.insert(output->name());

  // If the output object is a FileOutputBase then store the output filename
  FileOutputBase * ptr = dynamic_cast<FileOutputBase *>(output);
  if (ptr != NULL)
    addOutputFilename(ptr->filename());

  // Warning if multiple Console objects are added
  Console * c_ptr = dynamic_cast<Console *>(output);
  if (c_ptr != NULL && _has_console)
    mooseWarning("Multiple Console objects exists, this will likely cause duplicate messages printed to the screen");
  else if (c_ptr != NULL)
    _has_console = true;
}


bool
OutputWarehouse::hasOutput(std::string name)
{
  return _output_names.find(name) != _output_names.end();
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
OutputWarehouse::output()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->output();
}

void
OutputWarehouse::meshChanged()
{
  for (std::vector<OutputBase *>::const_iterator it = _object_ptrs.begin(); it != _object_ptrs.end(); ++it)
    (*it)->meshChanged();
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
