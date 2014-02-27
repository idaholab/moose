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

#ifndef FILEOUTPUTINTERFACE_H
#define FILEOUTPUTINTERFACE_H

// Moose includes
#include "InputParameters.h"

// Forward declerations
class FileOutputInterface;

template<>
InputParameters validParams<FileOutputInterface>();

/**
 * A stand-alone Interface class for adding basic filename support to output objects
 *
 * @see Exodus
 */
class FileOutputInterface
{
public:

  /**
   * Class constructor
   */
  FileOutputInterface(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~FileOutputInterface();

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() = 0;

protected:

  /// The base filename from the input paramaters
  std::string _file_base;

  /// Number of digits to pad the extensions
  unsigned int _padding;

};

#endif /* FILEOUTPUTINTERFACE_H */
