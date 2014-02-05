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

#ifndef FILEOUTPUTBASE_H
#define FILEOUTPUTBASE_H

// Moose includes
#include "InputParameters.h"

// Forward declerations
class FileOutputBase;

template<>
InputParameters validParams<FileOutputBase>();

/**
 * A stand-alone Base class for adding basic filename support to output objects
 *
 * @see Exodus
 */
class FileOutputBase
{
public:

  /**
   * Class constructor
   */
  FileOutputBase(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~FileOutputBase();

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() = 0;

protected:

  /// The base filename from the input paramaters
  OutFileBase _file_base;

};

#endif /* FILEOUTPUTBASE_H */
