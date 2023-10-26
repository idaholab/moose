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

  // Since it makes the most sense to write checkpoints at the end of time steps,
  // change the default value of execute_on to TIMESTEP_END
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum = {EXEC_TIMESTEP_END};

  return params;
}

Checkpoint::Checkpoint(const InputParameters & parameters)
  : FileOutput(parameters),
    _is_autosave(getParam<AutosaveType>("is_autosave")),
    _num_files(getParam<unsigned int>("num_files")),
    _suffix(getParam<std::string>("suffix"))
{
  // Prevent the checkpoint from executing at any time other than INITIAL and/or
  // TIMESTEP_END
  const auto & execute_on = getParam<ExecFlagEnum>("execute_on");

  // Create a vector containing all valid values of execute_on
  std::vector<ExecFlagEnum> valid_execute_on_values(3);
  {
    ExecFlagEnum valid_execute_on_value = execute_on;
    valid_execute_on_value.clear();
    valid_execute_on_value += EXEC_INITIAL;
    valid_execute_on_values[0] = valid_execute_on_value;
    valid_execute_on_value += EXEC_TIMESTEP_END;
    valid_execute_on_values[1] = valid_execute_on_value;
    valid_execute_on_value.clear();
    valid_execute_on_value += EXEC_TIMESTEP_END;
    valid_execute_on_values[2] = valid_execute_on_value;
  }

  // Check if the value of execute_on is valid
  auto it = std::find(valid_execute_on_values.begin(), valid_execute_on_values.end(), execute_on);
  const bool is_valid_value = (it != valid_execute_on_values.end());
  if (!is_valid_value)
    paramError("execute_on",
               "The checkpoint system may only be used with execute_on values ",
               "INITIAL and/or TIMESTEP_END, not '",
               execute_on,
               "'.");
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
  const bool parent_should_output = FileOutput::shouldOutput();
  // Check if the checkpoint should "normally" output, i.e. if it was created
  // through checkpoint=true
  bool should_output =
      (onInterval() || _current_execute_flag == EXEC_FINAL) ? parent_should_output : false;

  // If this is either a auto-created checkpoint, or if its an existing
  // checkpoint acting as the autosave and that checkpoint isn't on its
  // interval, then output.
  // parent_should_output ensures that we output only when _execute_on contains
  // _current_execute_flag (see Output::shouldOutput), ensuring that we wait
  // until the end of the timestep to write, preventing the output of an
  // unconverged solution.
  if (parent_should_output &&
      (_is_autosave == SYSTEM_AUTOSAVE || (_is_autosave == MODIFIED_EXISTING && !should_output)))
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
  const auto cp_dir = directory();
  Utility::mkdir(cp_dir.c_str());

  // Create the output filename
  const auto current_file = filename();

  // Create the libMesh Checkpoint_IO object
  MeshBase & mesh = _es_ptr->get_mesh();
  CheckpointIO io(mesh, true);

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
  // Update the list of stored files
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
