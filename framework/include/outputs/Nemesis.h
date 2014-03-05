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

#ifndef NEMESIS_H
#define NEMESIS_H

// MOOSE includes
#include "OversampleOutputter.h"

// libMesh includes
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"

// Forward declearations
class Nemesis;

template<>
InputParameters validParams<Nemesis>();

/**
 * Class for output data to the Nemesis format
 */
class Nemesis :
  public OversampleOutputter
{
public:

  /**
   * Class consturctor
   */
  Nemesis(const std::string & name, InputParameters);

  /**
   * Class destructor
   */
  virtual ~Nemesis();

  /**
   * Overload the OutputBase::output method, this is required for Nemesis
   * output due to the method utlized for outputing single/global parameters
   */
  virtual void output();

  /**
   * Sets up the libMesh::NemesisII_IO object used for outputting to the Nemesis format
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
   * Returns the current filename, this method handles the -s000 suffix
   * common to NemesisII files.
   * @return A string containg the current filename to be written
   */
  std::string filename();

  /// Pointer to the libMesh::NemesisII_IO object that performs the actual data output
  Nemesis_IO * _nemesis_io_ptr;

  /// Storage for scalar values (postprocessors and scalar AuxVariables)
  std::vector<Real> _global_values;

  /// Storage for names of the above scalar values
  std::vector<std::string> _global_names;

  /// Current output filename; utlized by filename() to create the proper suffix
  unsigned int _file_num;

private:

  /// Count of outputs per exodus file
  unsigned int _nemesis_num;

};

#endif /* NEMESIS_H */
