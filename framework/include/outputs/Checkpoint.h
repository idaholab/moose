//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FileOutput.h"
#include "RestartableDataIO.h"

#include <deque>

class MaterialPropertyStorage;

/**
 * A structure for storing the various output files associated with checkpoint output
 */
struct CheckpointFileNames
{
  /// Filename for CheckpointIO file
  std::string checkpoint;

  /// Filename for EquationsSystems::write
  std::string system;

  /// Filename for restartable data filename
  std::string restart;

  /// Filename for restartable data filename
  std::set<std::string> restart_meta_data;
};

/**
 *
 */
class Checkpoint : public FileOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters
   */
  Checkpoint(const InputParameters & parameters);

  /**
   * Returns the base filename for the checkpoint files
   */
  virtual std::string filename() override;

  /**
   * Retrieve the checkpoint output directory
   * @return String containing the checkpoint output directory
   */
  std::string directory() const;

  /**
   * Method to return the file suffix (ASCII or binary) for the Checkpoint format.
   */
  std::string getMeshFileSuffix(bool is_binary)
  {
    return is_binary ? BINARY_MESH_SUFFIX : ASCII_MESH_SUFFIX;
  }

protected:
  /**
   * Outputs a checkpoint file.
   * Each call to this function creates various files associated with
   */
  virtual void output(const ExecFlagType & type) override;

private:
  void updateCheckpointFiles(CheckpointFileNames file_struct);

  bool _should_output;

  bool _is_autosave;
  /// Max no. of output files to store
  unsigned int _num_files;

  /// Directory suffix
  const std::string _suffix;

  /// True if outputting checkpoint files in binary format
  bool _binary;

  /// True if running with parallel mesh
  bool _parallel_mesh;

  /// Reference to the restartable data
  const RestartableDataMaps & _restartable_data;

  /// RestrableData input/output interface
  RestartableDataIO _restartable_data_io;

  /// Vector of checkpoint filename structures
  std::deque<CheckpointFileNames> _file_names;

  static constexpr auto ASCII_MESH_SUFFIX = "_mesh.cpa";
  static constexpr auto BINARY_MESH_SUFFIX = "_mesh.cpr";
};
