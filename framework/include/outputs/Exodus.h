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
#include "OversampleOutput.h"

// libMesh forward declarations
namespace libMesh
{
class ExodusII_IO;
}

/**
 * Class for output data to the ExodusII format
 */
class Exodus : public OversampleOutput
{
public:
  static InputParameters validParams();

  enum class OutputDimension : int
  {
    DEFAULT,
    ONE,
    TWO,
    THREE,
    PROBLEM_DIMENSION
  };

  /**
   * Class constructor
   */
  Exodus(const InputParameters & parameters);

  /**
   * Overload the OutputBase::output method, this is required for ExodusII
   * output due to the method utilized for outputting single/global parameters
   */
  virtual void output() override;

  /**
   * Performs basic error checking and initial setup of ExodusII_IO output object
   */
  virtual void initialSetup() override;

  /**
   * Set flag indicating that the mesh has changed
   */
  virtual void meshChanged() override;

  /**
   * Performs the necessary deletion and re-creating of ExodusII_IO object
   *
   * This function is stand-alone and called directly from the output() method because
   * the ExodusII_IO object is extremely fragile with respect to closing a file that has
   * not had data written. Thus, it is important to only create a new ExodusII_IO object
   * if it is certain that it will be used.
   */
  virtual void outputSetup();

  /**
   * Set the sequence state
   * When the sequence state is set to true then the outputSetup() method is called with every
   * call to output(). In the case of Exodus output, this creates a new file with each output.
   *
   * The sequence state is automatically set to true when 'use_displaced = true', otherwise it
   * is set to false initially
   */
  virtual void sequence(bool state);

  /**
   * Force the output dimension programatically
   *
   * @param dim The dimension written in the output file
   */
  void setOutputDimension(unsigned int dim);

  /**
   * Helper method to change the output dimension in the passed in Exodus writer depending on
   * the dimension and coordinates of the passed in mesh.
   *
   * @param exodus_io The ExodusII_IO object to modify
   * @param mesh The MooseMesh object that is queried to determine the appropriate output dimension.
   */
  static void
  setOutputDimensionInExodusWriter(libMesh::ExodusII_IO & exodus_io,
                                   const MooseMesh & mesh,
                                   OutputDimension output_dim = OutputDimension::DEFAULT);

  /// Reset Exodus output
  void clear();

protected:
  /**
   * Outputs nodal, nonlinear variables
   */
  virtual void outputNodalVariables() override;

  /**
   * Outputs elemental, nonlinear variables
   */
  virtual void outputElementalVariables() override;

  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors() override;

  /**
   * Writes scalar AuxVariables to global output parameters
   */
  virtual void outputScalarVariables() override;

  /**
   * Writes the input file to the ExodusII output
   */
  virtual void outputInput() override;

  /**
   * Writes the Reporter values to the ExodusII output
   */
  virtual void outputReporters() override;

  /**
   * Customizes file output settings.
   */
  virtual void customizeFileOutput();

  /**
   * Returns the current filename, this method handles the -s000 suffix
   * common to ExodusII files.
   * @return A string containing the current filename to be written
   */
  virtual std::string filename() override;

  /// Pointer to the libMesh::ExodusII_IO object that performs the actual data output
  std::unique_ptr<libMesh::ExodusII_IO> _exodus_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /**
   * Flag for indicating the status of the ExodusII file that is being written. The ExodusII_IO
   * interface requires that the file be 'initialized' prior to writing any type of data. This
   * initialization occurs when write_timestep() is called. However, write_timestep also writes
   * nodal
   * data, so in the case where no nodal data is output, it is necessary to call write_timestep()
   * after
   * calling set_output_variables with an empty input string. This flag allows for the various
   * output
   * methods to check that the ExodusII file is in the proper state prior to writing data.
   * @see outputEmptyTimestep()
   */
  bool _exodus_initialized;

  /// A flag indicating to the Exodus object that the mesh has changed
  bool & _exodus_mesh_changed;

  /// Sequence flag, if true each timestep is written to a new file
  bool _sequence;

  /// Count of outputs per exodus file
  unsigned int & _exodus_num;

private:
  /// Handle the call to mesh renumbering in libmesh's ExodusIO on non-contiguously numbered meshes
  void handleExodusIOMeshRenumbering();

  /**
   * A helper function for 'initializing' the ExodusII output file, see the comments for the
   * _initialized
   * member variable.
   * @see _initialized
   */
  void outputEmptyTimestep();

  /// Flag indicating MOOSE is recovering via --recover command-line option
  bool _recovering;

  /// Storage for input file record; this is written to the file only after it has been initialized
  std::vector<std::string> _input_record;

  /// Flag for overwriting timesteps
  bool _overwrite;

  /// Enum for the output dimension
  OutputDimension _output_dimension;

  /// Flag to output discontinuous format in Exodus
  bool _discontinuous;

  /// Flag to output added disjoint fictitious sides for side_discontinuous variables
  bool _side_discontinuous;

  /// Flag to output HDF5 format (when available) in Exodus
  bool _write_hdf5;

  /// whether the mesh is contiguously numbered (exodus output will force that)
  bool _mesh_contiguous_numbering;
};
