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
#include "OutputBase.h"
#include "FileOutputInterface.h"
#include "MaterialPropertyStorage.h"
#include "RestartableData.h"
#include "MaterialPropertyIO.h"
#include "RestartableDataIO.h"

// Forward declerations
class Checkpoint;
struct CheckpointFileNames;

template<>
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

  /// Filename for stateful material property file
  std::string material;

  /// Filename for restartable data filename
  std::string restart;
};

/**
 *
 */
class Checkpoint:
  public OutputBase,
  public FileOutputInterface
{
public:

  /**
   * Class constructor
   * @param name
   * @param parameters
   */
  Checkpoint(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~Checkpoint();

  /**
   * Outputs a checkpoint file.
   * Each call to this function creates various files associated with
   */
  void output();

  /**
   * Returns the base filename for the checkpoint files
   */
  std::string filename();

protected:

  //@{
  /**
   * Invalid for Checkpoint output
   */
  virtual void outputNodalVariables();
  virtual void outputElementalVariables();
  virtual void outputScalarVariables();
  virtual void outputPostprocessors();
  //@}

  void updateCheckpointFiles(CheckpointFileNames file_struct);

private:

  /**
   * Retrieve the checkpoint output directory
   * @return String containing the checkpoint output directory
   */
  std::string directory();

  /// Max no. of output files to store
  unsigned int _num_files;

  /// Directory suffix
  const std::string _suffix;

  /// True if outputing checkpoint files in binary format
  bool _binary;

  /// Reference to the restartable data
  const RestartableDatas & _restartable_data;

  /// Reference to the recoverable data
  std::set<std::string> & _recoverable_data;

  /// Reference to the material property storage
  const MaterialPropertyStorage & _material_property_storage;

  /// MaterialProperty input/output interface
  MaterialPropertyIO _material_property_io;

  /// RestrableData input/output interface
  RestartableDataIO _restartable_data_io;

  /// Vector of checkpoint filename structures
  std::vector<CheckpointFileNames> _file_names;
};

#endif //CHECKPOINT_H
