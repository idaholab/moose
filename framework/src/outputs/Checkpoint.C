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

// Moose includes
#include "Checkpoint.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MaterialPropertyStorage.h"
#include "RestartableData.h"
#include "MooseMesh.h"

#include "libmesh/checkpoint_io.h"
#include "libmesh/enum_xdr_mode.h"

template <>
InputParameters
validParams<Checkpoint>()
{
  // Get the parameters from the base classes
  InputParameters params = validParams<FileOutput>();

  // Typical checkpoint options
  params.addParam<unsigned int>("num_files", 2, "Number of the restart files to save");
  params.addParam<std::string>(
      "suffix",
      "cp",
      "This will be appended to the file_base to create the directory name for checkpoint files.");

  // Advanced settings
  params.addParam<bool>("binary", true, "Toggle the output of binary files");
  params.addParamNamesToGroup("binary", "Advanced");
  return params;
}

Checkpoint::Checkpoint(const InputParameters & parameters)
  : FileOutput(parameters),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix")),
    _binary(getParam<bool>("binary")),
    _parallel_mesh(_problem_ptr->mesh().isDistributedMesh()),
    _restartable_data(_app.getRestartableData()),
    _recoverable_data(_app.getRecoverableData()),
    _material_property_storage(_problem_ptr->getMaterialPropertyStorage()),
    _bnd_material_property_storage(_problem_ptr->getBndMaterialPropertyStorage()),
    _restartable_data_io(RestartableDataIO(*_problem_ptr))
{
}

std::string
Checkpoint::filename()
{
  // Get the time step with correct zero padding
  std::ostringstream output;
  output << directory() << "/" << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
         << std::right << timeStep();
  return output.str();
}

std::string
Checkpoint::directory()
{
  return _file_base + "_" + _suffix;
}

void
Checkpoint::output(const ExecFlagType & /*type*/)
{
  // Start the performance log
  Moose::perf_log.push("Checkpoint::output()", "Output");

  // Create the output directory
  std::string cp_dir = directory();
  mkdir(cp_dir.c_str(), S_IRWXU | S_IRGRP);

  // Create the output filename
  std::string current_file = filename();

  // Create the libMesh Checkpoint_IO object
  MeshBase & mesh = _es_ptr->get_mesh();
  CheckpointIO io(mesh, _binary);

  // Set libHilbert renumbering flag to false.  We don't support
  // N-to-M restarts regardless, and if we're *never* going to do
  // N-to-M restarts then libHilbert is just unnecessary computation
  // and communication.
  const bool renumber = false;

  // Create checkpoint file structure
  CheckpointFileNames current_file_struct;
  if (_binary)
  {
    current_file_struct.checkpoint = current_file + "_mesh.cpr";
    current_file_struct.system = current_file + ".xdr";
  }
  else
  {
    current_file_struct.checkpoint = current_file + "_mesh.cpa";
    current_file_struct.system = current_file + ".xda";
  }
  current_file_struct.restart = current_file + ".rd";

  // Write the checkpoint file
  io.write(current_file_struct.checkpoint);

  // Write the system data, using ENCODE vs WRITE based on xdr vs xda
  _es_ptr->write(current_file_struct.system,
                 EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA |
                     EquationSystems::WRITE_PARALLEL_FILES,
                 renumber);

  // Write the restartable data
  _restartable_data_io.writeRestartableData(
      current_file_struct.restart, _restartable_data, _recoverable_data);

  // Remove old checkpoint files
  updateCheckpointFiles(current_file_struct);

  // Stop the logging
  Moose::perf_log.pop("Checkpoint::output()", "Output");
}

void
Checkpoint::updateCheckpointFiles(CheckpointFileNames file_struct)
{
  // Update the list of stored files
  _file_names.push_back(file_struct);

  // Remove un-wanted files
  if (_file_names.size() > _num_files)
  {
    // Extract the filenames to be removed
    CheckpointFileNames delete_files = _file_names.front();

    // Remove these filenames from the list
    _file_names.pop_front();

    // Get thread and proc information
    processor_id_type proc_id = processor_id();

    // Delete checkpoint files (_mesh.cpr)
    if (proc_id == 0)
    {
      std::ostringstream oss;
      oss << delete_files.checkpoint;
      std::string file_name = oss.str();
      int ret = remove(file_name.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
    }

    if (_parallel_mesh)
    {
      std::ostringstream oss;
      oss << delete_files.checkpoint << '-' << n_processors() << '-' << proc_id;
      std::string file_name = oss.str();
      int ret = remove(file_name.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
    }
    else
    {
      if (proc_id == 0)
      {
        std::ostringstream oss;
        oss << delete_files.checkpoint << "-1-0";
        std::string file_name = oss.str();
        int ret = remove(file_name.c_str());
        if (ret != 0)
          mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
      }
    }

    // Delete the system files (xdr and xdr.0000, ...)
    if (proc_id == 0)
    {
      std::ostringstream oss;
      oss << delete_files.system;
      std::string file_name = oss.str();
      int ret = remove(file_name.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
    }
    {
      std::ostringstream oss;
      oss << delete_files.system << "." << std::setw(4) << std::setprecision(0) << std::setfill('0')
          << proc_id;
      std::string file_name = oss.str();
      int ret = remove(file_name.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
    }

    unsigned int n_threads = libMesh::n_threads();

    // Remove the restart files (rd)
    {
      for (THREAD_ID tid = 0; tid < n_threads; tid++)
      {
        std::ostringstream oss;
        oss << delete_files.restart << "-" << proc_id;
        if (n_threads > 1)
          oss << "-" << tid;
        std::string file_name = oss.str();
        int ret = remove(file_name.c_str());
        if (ret != 0)
          mooseWarning("Error during the deletion of file '", file_name, "': ", std::strerror(ret));
      }
    }
  }
}
