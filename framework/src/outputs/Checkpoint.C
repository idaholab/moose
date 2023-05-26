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
  // auto-checkpoint created by AutoCheckpointAction, which does not write unless a signal
  // is received.
  params.addPrivateParam<AutosaveType>("is_autosave", NONE);

  params.addClassDescription("Output for MOOSE recovery checkpoint files.");

  // Typical checkpoint options
  params.addParam<unsigned int>("num_files", 2, "Number of the restart files to save");
  params.addParam<std::string>(
      "suffix",
      "cp",
      "This will be appended to the file_base to create the directory name for checkpoint files.");

  return params;
}

Checkpoint::Checkpoint(const InputParameters & parameters)
  : FileOutput(parameters),
    _is_autosave(getParam<AutosaveType>("is_autosave")),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix")),
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
Checkpoint::outputStep(const ExecFlagType & type)
{
  // Output is not allowed
  if (!_allow_output && type != EXEC_FORCED)
    return;

  // If recovering disable output of initial condition, it was already output
  if (type == EXEC_INITIAL && _app.isRecovering())
    return;

  // store current simulation time
  _last_output_time = _time;

  // set current type
  _current_execute_flag = type;

  // Check whether we should output, then do it.
  if (shouldOutput())
  {
    TIME_SECTION("outputStep", 2, "Outputting Checkpoint");
    output();
  }

  _current_execute_flag = EXEC_NONE;
}

bool
Checkpoint::shouldOutput()
{
  // Check if the checkpoint should "normally" output, i.e. if it was created
  // through checkpoint=true
  bool should_output =
      (onInterval() || _current_execute_flag == EXEC_FINAL) ? FileOutput::shouldOutput() : false;

  // If this is either a auto-created checkpoint, or if its an existing checkpoint acting
  // as the autosave and that checkpoint isn't on its interval, then output.
  if (_is_autosave == SYSTEM_AUTOSAVE || (_is_autosave == MODIFIED_EXISTING && !should_output))
  {
    // If this is a pure system-created autosave through AutoCheckpointAction,
    // then sync across processes and only output one time per signal received.
    comm().max(Moose::interrupt_signal_number);
    // Reading checkpoint on time step 0 is not supported
    should_output = (Moose::interrupt_signal_number != 0) && (timeStep() > 0);
    if (should_output)
    {
      _console << "Unix signal SIGUSR1 detected. Outputting checkpoint file. \n";
      // Reset signal number since we output
      Moose::interrupt_signal_number = 0;
    }
  }
  return should_output;
}

void
Checkpoint::output()
{
  // Create the output directory
  std::string cp_dir = directory();
  Utility::mkdir(cp_dir.c_str());

  // Create the output filename
  std::string current_file = filename();

  // Create the libMesh Checkpoint_IO object
  MeshBase & mesh = _es_ptr->get_mesh();
  CheckpointIO io(mesh, true);

  // Create checkpoint file structure
  CheckpointFileNames curr_file_struct;

  curr_file_struct.checkpoint = current_file + meshSuffix();
  curr_file_struct.restart = current_file + restartSuffix(processor_id());

  // Write the checkpoint file
  io.write(curr_file_struct.checkpoint);

  // Write out meta data if there is any (only on processor zero)
  if (processor_id() == 0)
  {
    for (auto & map_pair :
         libMesh::as_range(_app.getRestartableDataMapBegin(), _app.getRestartableDataMapEnd()))
    {
      const RestartableDataMap & meta_data = map_pair.second.first;
      const std::string & suffix = map_pair.second.second;
      const std::string filename(current_file + fullMetaDataSuffix(suffix));

      curr_file_struct.restart_meta_data.emplace(filename);
      _restartable_data_io.writeRestartableData(filename, meta_data);
    }
  }

  // Write out the backup
  auto backup = _app.backup();
  std::ofstream backup_file;
  const std::string backup_filename(curr_file_struct.restart);
  backup_file.open(backup_filename, std::ios::out | std::ios::binary);
  dataStore(backup_file, backup, nullptr);
  backup_file.close();

  // Remove old checkpoint files
  updateCheckpointFiles(curr_file_struct);
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

    // Get proc information
    processor_id_type proc_id = processor_id();

    // Delete meta data and checkpoint files
    if (proc_id == 0)
    {
      // Delete meta data files
      for (const auto & file_name : delete_files.restart_meta_data)
        remove(file_name.c_str());

      // This file may not exist so don't worry about checking for success
      CheckpointIO::cleanup(delete_files.checkpoint,
                            _problem_ptr->mesh().isDistributedMesh() ? comm().size() : 1);
    }

    // Delete restartable data
    {
      int ret = remove(delete_files.restart.c_str());
      if (ret != 0)
        mooseWarning(
            "Error during the deletion of file '", delete_files.restart, "': ", std::strerror(ret));
    }
  }
}

std::string
Checkpoint::meshSuffix()
{
  return "-mesh.cpr";

}

std::string
Checkpoint::metaDataSuffix(const std::string & suffix)
{
  return "/meta_data" + suffix + ".rd";
}

std::string
Checkpoint::fullMetaDataSuffix(const std::string & suffix)
{
  return meshSuffix() + "/meta_data" + suffix + ".rd";
}

std::string
Checkpoint::meshMetadataSuffix()
{
  return metaDataSuffix("_" + MooseApp::MESH_META_DATA_SUFFIX);
}

std::string
Checkpoint::fullMeshMetadataSuffix()
{
  return fullMetaDataSuffix("_" + MooseApp::MESH_META_DATA_SUFFIX);
}

std::string
Checkpoint::restartSuffix(const processor_id_type pid)
{
  return "-restart-" + std::to_string(pid) + ".rd";
}
