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

// C POSIX includes
#include <sys/stat.h>

// Moose includes
#include "Checkpoint.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MaterialPropertyStorage.h"
#include "RestartableData.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/checkpoint_io.h"
#include "libmesh/enum_xdr_mode.h"

template <>
InputParameters
validParams<Checkpoint>()
{
  // Get the parameters from the base classes
  InputParameters params = validParams<BasicOutput<FileOutput>>();

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
  : BasicOutput<FileOutput>(parameters),
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

  // Set renumbering flag (renumber if adaptivity is on)
  bool renumber = false;
  if (_problem_ptr->adaptivity().isOn())
    renumber = true;

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

  // Write the xdr
  _es_ptr->write(current_file_struct.system,
                 ENCODE,
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
  int ret = 0; // return code for file operations

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
    if (_parallel_mesh)
    {
      std::ostringstream oss;
      oss << delete_files.checkpoint << '-' << n_processors() << '-' << proc_id;
      ret = remove(oss.str().c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", oss.str().c_str(), "': ", ret);
    }
    else if (proc_id == 0)
    {
      ret = remove(delete_files.checkpoint.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", delete_files.checkpoint, "': ", ret);

      // Delete the system files (xdr and xdr.0000, ...)
      ret = remove(delete_files.system.c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", delete_files.system, "': ", ret);
    }

    {
      std::ostringstream oss;
      oss << delete_files.system << "." << std::setw(4) << std::setprecision(0) << std::setfill('0')
          << proc_id;
      ret = remove(oss.str().c_str());
      if (ret != 0)
        mooseWarning("Error during the deletion of file '", oss.str().c_str(), "': ", ret);
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
        ret = remove(oss.str().c_str());
        if (ret != 0)
          mooseWarning("Error during the deletion of file '", oss.str().c_str(), "': ", ret);
      }
    }
  }
}
