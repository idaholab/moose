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

#ifndef EXODUS_H
#define EXODUS_H

// MOOSE includes
#include "AdvancedOutput.h"
#include "OversampleOutput.h"

// libMesh includes
#include "libmesh/exodusII.h"
#include "libmesh/exodusII_io.h"

// Forward declarations
class Exodus;

template<>
InputParameters validParams<Exodus>();

/**
 * Class for output data to the ExodusII format
 */
class Exodus : public AdvancedOutput<OversampleOutput>
{
public:

  /**
   * Class constructor
   */
  Exodus(const std::string & name, InputParameters);

  /**
   * Class destructor
   */
  virtual ~Exodus();

  /**
   * Overload the OutputBase::output method, this is required for ExodusII
   * output due to the method utilized for outputing single/global parameters
   */
  virtual void output(const ExecFlagType & type);

  /**
   * Performs basic error checking and initial setup of ExodusII_IO output object
   */
  virtual void initialSetup();

  /**
   * Set flag indicating that the mesh has changed
   */
  virtual void meshChanged();

  /**
   * Performs the necessary deletion and re-creating of ExodusII_IO object
   *
   * This function is stand-alone and called directly from the output() method because
   * the ExodusII_IO object is extremely fragile with respect to closing a file that has
   * not had data written. Thus, it is important to only create a new ExodusII_IO object
   * if it is certain that it will be used.
   */
  void outputSetup();

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
   * Return the time for writting to the file
   */
  virtual Real time();

protected:

  /**
   * Outputs nodal, nonlinear variables
   */
  virtual void outputNodalVariables();

  /**
   * Outputs elemental, nonlinear variables
   */
  virtual void outputElementalVariables();

  /**
   * Writes postprocessor values to global output parameters
   */
  virtual void outputPostprocessors();

  /**
   * Writes scalar AuxVariables to global output parameters
   */
  virtual void outputScalarVariables();

  /**
   * Writes the input file to the ExodusII output
   */
  virtual void outputInput();

  /**
   * Returns the current filename, this method handles the -s000 suffix
   * common to ExodusII files.
   * @return A string containing the current filename to be written
   */
  std::string filename();

  /// Pointer to the libMesh::ExodusII_IO object that performs the actual data output
  MooseSharedPointer<ExodusII_IO> _exodus_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /**
   * Flag for indicating the status of the ExodusII file that is being written. The ExodusII_IO
   * interface requires that the file be 'initialized' prior to writing any type of data. This
   * initialization occurs when write_timestep() is called. However, write_timestep also writes nodal
   * data, so in the case where no nodal data is output, it is necessary to call write_timestep() after
   * calling set_output_variables with an empty input string. This flag allows for the various output
   * methods to check that the ExodusII file is in the proper state prior to writing data.
   * @see outputEmptyTimestep()
   */
  bool _exodus_initialized;


private:

  /**
   * A helper function for 'initializing' the ExodusII output file, see the comments for the _initialized
   * member variable.
   * @see _initialized
   */
  void outputEmptyTimestep();

  /// Count of outputs per exodus file
  unsigned int & _exodus_num;

  /// Flag indicating MOOSE is recovering via --recover command-line option
  bool _recovering;

  /// Storage for input file record; this is written to the file only after it has been initialized
  std::vector<std::string> _input_record;

  /// A flag indicating to the Exodus object that the mesh has changed
  bool & _exodus_mesh_changed;

  /// Sequence flag, if true each timestep is written to a new file
  bool _sequence;

  /// Flag for using EnSignt compatible time
  bool _ensight_time;
};

#endif /* EXODUS_H */
