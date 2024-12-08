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
#include "AdvancedOutput.h"

// libMesh forward declarations
namespace libMesh
{
class Nemesis_IO;
}

/**
 * Class for output data to the Nemesis format
 */
class Nemesis : public AdvancedOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  Nemesis(const InputParameters & parameters);

  /**
   * Sets up the libMesh::NemesisII_IO object used for outputting to the Nemesis format
   */
  virtual void initialSetup() override;

  /**
   * Creates a new NemesisII_IO output object for outputting a new mesh
   */
  virtual void meshChanged() override;

  /**
   * Performs the necessary deletion and re-creating of NemesisII_IO object
   *
   * This function is stand-alone and called directly from the output() method because
   * the NemesisII_IO object is extremely fragile with respect to closing a file that has
   * not had data written. Thus, it is important to only create a new NemesisII_IO object
   * if it is certain that it will be used.
   */
  virtual void outputSetup();

protected:
  /**
   * Overload the Output::output method, this is required for Nemesis
   * output due to the method utilized for outputting single/global parameters
   */
  virtual void output() override;

  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors() override;

  /**
   * Writes scalar AuxVariables to global output parameters
   */
  virtual void outputScalarVariables() override;

  /**
   * Returns the current filename, this method handles the -s000 suffix
   * common to NemesisII files.
   * @return A string containing the current filename to be written
   */
  virtual std::string filename() override;

  /// Pointer to the libMesh::NemesisII_IO object that performs the actual data output
  std::unique_ptr<libMesh::Nemesis_IO> _nemesis_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /// Current output filename; utilized by filename() to create the proper suffix
  unsigned int & _file_num;

private:
  /// Count of outputs per exodus file
  unsigned int & _nemesis_num;

  /// Flag if the output has been initialized
  bool _nemesis_initialized;

  /// Flag indicating MOOSE is recovering via --recover command-line option
  bool _recovering;

  /// A flag indicating to the Nemesis object that the mesh has changed
  bool & _nemesis_mesh_changed;

  /// Flag to output HDF5 format (when available) in Nemesis. libMesh wants to do
  ///  so by default (for backwards compatibility with libMesh HDF5 users), but we
  /// want to avoid this by default (for backwards compatibility with most Moose users
  /// and to avoid generating regression test gold files that non-HDF5 Moose builds can't read)
  bool _write_hdf5;
};
