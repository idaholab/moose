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
#include "MeshMetaDataInterface.h"

#include "libmesh/checkpoint_io.h"
#include "libmesh/enum_xdr_mode.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", Checkpoint);

InputParameters
Checkpoint::validParams()
{
  // Get the parameters from the base classes
  InputParameters params = FileOutput::validParams();

  // Controls whether the checkpoint will actually run. Should only ever be changed by the
  // auto-checkpoint created by the signal handler, which does not write unless a signal is
  // received.
  params.addPrivateParam<bool>("should_output", true);
  params.addPrivateParam<bool>("is_autosave", false);

  params.addClassDescription("Output for MOOSE recovery checkpoint files.");

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
    _should_output(getParam<bool>("should_output")),
    _is_autosave(getParam<bool>("is_autosave")),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix")),
    _binary(getParam<bool>("binary")),
    _parallel_mesh(_problem_ptr->mesh().isDistributedMesh()),
    _restartable_data(_app.getRestartableData()),
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
Checkpoint::directory() const
{
  return _file_base + "_" + _suffix;
}

void
Checkpoint::output(const ExecFlagType & /*type*/)
{
  // Check if we should write the autosave checkpoint. The only time _should_output will
  // be false is if this Checkpoint object is created through AutoCheckpointAction, and if it
  // is false, then we check if the signal handler has set the flag for us to write it out.
  if (!_should_output)
  {
    comm().max(Moose::autosave_flag);
    _should_output = Moose::autosave_flag;
  }

  if (_should_output)
  {
    // Create the output directory
    std::string cp_dir = directory();
    Utility::mkdir(cp_dir.c_str());

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
    CheckpointFileNames curr_file_struct;

    curr_file_struct.checkpoint = current_file + getMeshFileSuffix(_binary);
    curr_file_struct.system = current_file + _restartable_data_io.getESFileExtension(_binary);
    curr_file_struct.restart = current_file + _restartable_data_io.getRestartableDataExt();

    // Write the checkpoint file
    io.write(curr_file_struct.checkpoint);

    // Write out the restartable mesh meta data if there is any (only on processor zero)
    if (processor_id() == 0)
    {
      for (auto & map_pair :
           libMesh::as_range(_app.getRestartableDataMapBegin(), _app.getRestartableDataMapEnd()))
      {
        const RestartableDataMap & meta_data = map_pair.second.first;
        const std::string & suffix = map_pair.second.second;
        const std::string filename(curr_file_struct.checkpoint + "/meta_data" + suffix +
                                   _restartable_data_io.getRestartableDataExt());
        curr_file_struct.restart_meta_data.emplace(filename);
        _restartable_data_io.writeRestartableData(filename, meta_data);
      }
    }

    // Write the system data, using ENCODE vs WRITE based on ascii vs binary format
    _es_ptr->write(curr_file_struct.system,
                   EquationSystems::WRITE_DATA | EquationSystems::WRITE_ADDITIONAL_DATA |
                       EquationSystems::WRITE_PARALLEL_FILES,
                   renumber);

    // Write the restartable data
    _restartable_data_io.writeRestartableDataPerProc(curr_file_struct.restart, _restartable_data);

    // Remove old checkpoint files
    updateCheckpointFiles(curr_file_struct);

    // Stop outputting the checkpoint if this is a signaled/autosave checkpoint
    if (Moose::autosave_flag and _is_autosave)
    {
      _should_output = false;
      Moose::autosave_flag = 0;
    }
  }
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
      for (const auto & file_name : delete_files.restart_meta_data)
        remove(file_name.c_str());
      // This file may not exist so don't worry about checking for success

      CheckpointIO::cleanup(delete_files.checkpoint, _parallel_mesh ? comm().size() : 1);

      // Delete the system files (xdr and xdr.0000, ...)
      const auto & file_name = delete_files.system;
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

    // Remove the restart files (rd)
    unsigned int n_threads = libMesh::n_threads();
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
