//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHECKPOINT_H
#define CHECKPOINT_H

// MOOSE includes
#include "FileOutput.h"
#include "RestartableDataIO.h"

#include <deque>

// Forward declarations
class Checkpoint;
class MaterialPropertyStorage;

template <>
InputParameters validParams<Checkpoint>();

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
};

/**
 *
 */
class Checkpoint : public FileOutput
{
public:
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
  std::string directory();

protected:
  /**
   * Outputs a checkpoint file.
   * Each call to this function creates various files associated with
   */
  virtual void output(const ExecFlagType & type) override;

private:
  void updateCheckpointFiles(CheckpointFileNames file_struct);

  /// Max no. of output files to store
  unsigned int _num_files;

  /// Directory suffix
  const std::string _suffix;

  /// True if outputing checkpoint files in binary format
  bool _binary;

  /// True if running with parallel mesh
  bool _parallel_mesh;

  /// Reference to the restartable data
  const RestartableDatas & _restartable_data;

  /// Reference to the recoverable data
  std::set<std::string> & _recoverable_data;

  /// Reference to the material property storage
  const MaterialPropertyStorage & _material_property_storage;

  /// Reference to the boundary material property storage
  const MaterialPropertyStorage & _bnd_material_property_storage;

  /// RestrableData input/output interface
  RestartableDataIO _restartable_data_io;

  /// Vector of checkpoint filename structures
  std::deque<CheckpointFileNames> _file_names;
};

#endif // CHECKPOINT_H
