//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// C POSIX includes
#include <sstream>
#include <sys/stat.h>

#include <system_error>

// Moose includes
#include "Checkpoint.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "MaterialPropertyStorage.h"
#include "MooseMesh.h"
#include "MeshMetaDataInterface.h"
#include "RestartableDataWriter.h"

#include "libmesh/checkpoint_io.h"
#include "libmesh/enum_xdr_mode.h"
#include "libmesh/utility.h"

using namespace libMesh;

registerMooseObject("MooseApp", Checkpoint);

InputParameters
Checkpoint::validParams()
{
  // Get the parameters from the base classes
  InputParameters params = FileOutput::validParams();

  // Controls whether the checkpoint will actually run. Should only ever be changed by the
  // auto-checkpoint created by AutoCheckpointAction, which does not write unless a signal
  // is received.
  params.addPrivateParam<CheckpointType>("checkpoint_type", CheckpointType::USER_CREATED);

  params.addClassDescription("Output for MOOSE recovery checkpoint files.");

  // Typical checkpoint options
  params.addParam<unsigned int>("num_files", 2, "Number of the restart files to save");
  params.addParam<std::string>(
      "suffix",
      "cp",
      "This will be appended to the file_base to create the directory name for checkpoint files.");
  // For checkpoints, set the wall time output interval to defualt of 1 hour (3600 s)
  params.addParam<Real>(
      "wall_time_interval", 3600, "The target wall time interval (in seconds) at which to output");

  // Parameter to turn off wall time checkpoints
  params.addParam<bool>(
      "wall_time_checkpoint", true, "Whether to enable checkpoints based on elapsed wall time");

  // Since it makes the most sense to write checkpoints at the end of time steps,
  // change the default value of execute_on to TIMESTEP_END
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_TIMESTEP_END};

  return params;
}

Checkpoint::Checkpoint(const InputParameters & parameters)
  : FileOutput(parameters),
    _checkpoint_type(getParam<CheckpointType>("checkpoint_type")),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix"))
{
  // Prevent the checkpoint from executing at any time other than INITIAL,
  // TIMESTEP_END, and FINAL
  validateExecuteOn();

  // The following updates the value of _wall_time_interval if the
  // '--output-wall-time-interval' command line parameter is used.
  // If it is not used, _wall_time_interval keeps its current value.
  // 'The --output-wall-time-interval parameter is necessary for testing
  // and should only be used in the test suite.
  Output::setWallTimeIntervalFromCommandLineParam();

  // We want to do this here so it overrides --output-wall-time-interval
  if (!getParam<bool>("wall_time_checkpoint"))
    _wall_time_interval = std::numeric_limits<Real>::max();
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

bool
Checkpoint::shouldOutput()
{
  // should_output_parent ensures that we output only when _execute_on contains
  // _current_execute_flag (see Output::shouldOutput), ensuring that we wait
  // until the end of the timestep to write, preventing the output of an
  // unconverged solution.
  const bool should_output_parent = FileOutput::shouldOutput();
  if (!should_output_parent)
    return false; // No point in continuing

  // Check for signal
  // Reading checkpoint on time step 0 is not supported
  const bool should_output_signal = (Moose::interrupt_signal_number != 0) && (timeStep() > 0);
  if (should_output_signal)
  {
    _console << "Unix signal SIGUSR1 detected. Outputting checkpoint file.\n";
    // Reset signal number since we output
    Moose::interrupt_signal_number = 0;
    return true;
  }

  // Check if enough wall time has elapsed to output
  const bool should_output_wall_time = _wall_time_since_last_output >= _wall_time_interval;
  if (should_output_wall_time)
    return true;

  // At this point, we have checked all automatic checkpoint options. If none
  // of those triggered, then the only way a checkpoint will still be written
  // is if the user defined it. If the checkpoint is purely system-created,
  // go ahead and return false (circumvents default time_step_interval = 1 for
  // auto checkpoints).
  if (_checkpoint_type == CheckpointType::SYSTEM_CREATED)
    return false;

  // Check if the checkpoint should "normally" output, i.e. if it was created
  // through the input file
  const bool should_output = (onInterval() || _current_execute_flag == EXEC_FINAL);

  return should_output;
}

void
Checkpoint::output()
{
  // Create the output directory
  const auto cp_dir = directory();
  Utility::mkdir(cp_dir.c_str());

  // Create the output filename
  const auto current_file = filename();

  // Create the libMesh Checkpoint_IO object
  MeshBase & mesh = _es_ptr->get_mesh();
  CheckpointIO io(mesh, false);

  // Create checkpoint file structure
  CheckpointFileNames curr_file_struct;

  curr_file_struct.checkpoint = current_file + _app.checkpointSuffix();

  // Write the checkpoint file
  io.write(curr_file_struct.checkpoint);

  // Write out meta data if there is any (only on processor zero)
  if (processor_id() == 0)
  {
    const auto paths = _app.writeRestartableMetaData(curr_file_struct.checkpoint);
    curr_file_struct.restart.insert(curr_file_struct.restart.begin(), paths.begin(), paths.end());
  }

  // Write out the backup
  const auto paths = _app.backup(_app.restartFolderBase(current_file));
  curr_file_struct.restart.insert(curr_file_struct.restart.begin(), paths.begin(), paths.end());

  // Remove old checkpoint files
  updateCheckpointFiles(curr_file_struct);
}

void
Checkpoint::updateCheckpointFiles(CheckpointFileNames file_struct)
{
  // It is possible to have already written a checkpoint with the same file
  // names contained in file_struct. If this is the case, file_struct will
  // already be stored in _file_names. When this happens, the current state of
  // the simulation is likely different than the state when the duplicately
  // named checkpoint was last written. Because of this, we want to go ahead and
  // rewrite the duplicately named checkpoint, overwritting the files
  // representing the old state. For accurate bookkeeping, we will delete the
  // existing instance of file_struct from _file_names and re-append it to the
  // end of _file_names (to keep the order in which checkpoints are written
  // accurate).

  const auto it = std::find(_file_names.begin(), _file_names.end(), file_struct);
  // file_struct was found in _file_names.
  // Delete it so it can be re-added as the last element.
  if (it != _file_names.end())
    _file_names.erase(it);

  _file_names.push_back(file_struct);

  // Remove the file and the corresponding directory if it's empty
  const auto remove_file = [this](const std::filesystem::path & path)
  {
    std::error_code err;

    if (!std::filesystem::remove(path, err))
      mooseWarning("Error during the deletion of checkpoint file\n",
                   std::filesystem::absolute(path),
                   "\n\n",
                   err.message());

    const auto dir = path.parent_path();
    if (std::filesystem::is_empty(dir))
      if (!std::filesystem::remove(dir, err))
        mooseError("Error during the deletion of checkpoint directory\n",
                   std::filesystem::absolute(dir),
                   "\n\n",
                   err.message());
  };

  // Remove un-wanted files
  if (_file_names.size() > _num_files)
  {
    // Extract the filenames to be removed
    CheckpointFileNames delete_files = _file_names.front();

    // Remove these filenames from the list
    _file_names.pop_front();

    // Delete restartable data
    for (const auto & path : delete_files.restart)
      remove_file(path);

    // Delete checkpoint files
    // This file may not exist so don't worry about checking for success
    if (processor_id() == 0)
      CheckpointIO::cleanup(delete_files.checkpoint,
                            _problem_ptr->mesh().isDistributedMesh() ? comm().size() : 1);
  }
}

void
Checkpoint::validateExecuteOn() const
{
  const auto & execute_on = getParam<ExecFlagEnum>("execute_on");
  const std::set<ExecFlagType> allowed = {EXEC_INITIAL, EXEC_TIMESTEP_END, EXEC_FINAL};
  for (const auto & value : execute_on)
    if (!allowed.count(value))
      paramError("execute_on",
                 "The exec flag ",
                 value,
                 " is not allowed. Allowed flags are INITIAL, TIMESTEP_END, and FINAL.");
}

std::stringstream
Checkpoint::checkpointInfo() const
{
  static const unsigned int console_field_width = 27;
  std::stringstream checkpoint_info;

  std::string interval_info;
  if (getParam<bool>("wall_time_checkpoint"))
  {
    std::stringstream interval_info_ss;
    interval_info_ss << "Every " << std::defaultfloat << _wall_time_interval << " s";
    interval_info = interval_info_ss.str();
  }
  else
    interval_info = "Disabled";

  checkpoint_info << std::left << std::setw(console_field_width)
                  << "  Wall Time Interval:" << interval_info << "\n";

  std::string user_info;
  if (_checkpoint_type == CheckpointType::SYSTEM_CREATED)
    user_info = "Disabled";
  else
    user_info = "Outputs/" + name();

  checkpoint_info << std::left << std::setw(console_field_width)
                  << "  User Checkpoint:" << user_info << "\n";

  if (!((interval_info == "Disabled") && (user_info == "Disabled")))
  {
    checkpoint_info << std::left << std::setw(console_field_width)
                    << "  # Checkpoints Kept:" << std::to_string(_num_files) << "\n";
    std::string exec_on_values = "";
    for (const auto & item : _execute_on)
      exec_on_values += item.name() + " ";
    checkpoint_info << std::left << std::setw(console_field_width)
                    << "  Execute On:" << exec_on_values << "\n";
  }

  return checkpoint_info;
}
