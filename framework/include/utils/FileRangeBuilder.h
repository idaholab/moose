/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FILERANGEBUILDER_H
#define FILERANGEBUILDER_H

// MOOSE includes
#include "InputParameters.h"

// Forward declarations
class FileRangeBuilder;

/**
 * To be called in the validParams functions of classes that need to
 * operate on ranges of files.  Adds several non-required parameters
 * that are parsed in the parseFileRange function.
 */
template <>
InputParameters validParams<FileRangeBuilder>();

/**
 * Augments an InputParameters object with file range information.
 * Creates and adds a vector<string> with the list of filenames to the
 * params object for use by the calling object.  The params object
 * passed in must contain suitable information for building the list
 * of filenames in the range.  Returns a non-zero error code if there
 * is an error while parsing.
 */
class FileRangeBuilder
{
public:
  FileRangeBuilder(const InputParameters & params);
  virtual ~FileRangeBuilder() = default;

  std::string fileSuffix() { return _file_suffix; }
  const std::vector<std::string> & filenames() { return _filenames; }

protected:
  //  int status(){ return _status; }
  void errorCheck();

  int _status;
  std::string _file_suffix;
  std::vector<std::string> _filenames;
};

#endif // FILERANGEBUILDER_H
