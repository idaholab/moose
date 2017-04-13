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

#ifndef CHECKPOINT_H
#define CHECKPOINT_H

// MOOSE includes
#include "BasicOutput.h"
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
class Checkpoint : public BasicOutput<FileOutput>
{
public:
  /**
   * Class constructor
   * @param parameters
   */
  Checkpoint(const InputParameters & parameters);

  /**
   * Outputs a checkpoint file.
   * Each call to this function creates various files associated with
   */
  void output(const ExecFlagType & type) override;

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
  void updateCheckpointFiles(CheckpointFileNames file_struct);

private:
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
