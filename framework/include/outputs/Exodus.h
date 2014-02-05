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
#include "FileOutputBase.h"
#include "OversampleBase.h"

// libMesh includes
#include "libmesh/exodusII.h"
#include "libmesh/exodusII_io.h"

// Forward declearations
class Exodus;

template<>
InputParameters validParams<Exodus>();

/**
 * Class for output data to the ExodusII format
 */
class Exodus :
  public OversampleBase,
  public FileOutputBase
{
public:

  /**
   * Class consturctor
   */
  Exodus(const std::string & name, InputParameters);

  /**
   * Class destructor
   */
  virtual ~Exodus();

  /**
   * Overload the OutputBase::output method, this is required for ExodusII
   * output due to the method utlized for outputing single/global parameters
   */
  virtual void output();

  /**
   * Sets up the libMesh::ExodusII_IO object used for outputting to the Exodus format
   */
  virtual void outputSetup();

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
   * @return A string containg the current filename to be written
   */
  std::string filename();

  /// Pointer to the libMesh::ExodusII_IO object that performs the actual data output
  ExodusII_IO * _exodus_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /// Current output filename; utlized by filename() to create the proper suffix
  unsigned int _file_num;

  /**
   * Flag for indicating the status of the ExodusII file that is being written. The ExodusII_IO
   * interface requires that the file be 'initialized' prior to writing any type of data. This
   * initialization occurs when write_timestep() is called. However, write_timestep also writes nodal
   * data, so in the case where no nodal data is output, it is necessary to call write_timestep() after
   * calling set_output_variables with an empty input string. This flag allows for the various output
   * methods to check that the ExodusII file is in the proper state prior to writing data.
   * @see outputEmptyTimestep()
   */
  bool _initialized;

private:

  /**
   * A helper function for 'initializing' the ExodusII output file, see the comments for the _initialized
   * member variable.
   * @see _initialized
   */
  void outputEmptyTimestep();

  /// Number of digits to pad the -s extensions
  unsigned int _padding;

  /// Count of outputs per exodus file
  unsigned int _exodus_num;

};

#endif /* EXODUS_H */
