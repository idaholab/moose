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
#include "Moose.h"

class InputParameters;

template <typename T>
InputParameters validParams();

/**
 * To be called in the validParams functions of classes that need to
 * operate on ranges of files.  Adds several non-required parameters
 * that are parsed in the parseFileRange function.
 */
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
  static InputParameters validParams();

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
