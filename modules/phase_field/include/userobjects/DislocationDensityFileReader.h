//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include <vector>

/**
 * Read a file to provide initial dislocation densities to grains
 **/
class DislocationDensityFileReader : public GeneralUserObject
{
public:
  DislocationDensityFileReader(const InputParameters & parameters);

  static InputParameters validParams();

  /// return the dislocation density
  virtual const Real & getDensity(unsigned int) const;

  /// get the grain ID
  virtual unsigned int getGrainNum() const;

  virtual void initialize() {}
  virtual void execute() {}
  virtual void finalize() {}

private:
  /// read the file for data
  void readFile();

  /// file name
  FileName _file_name;

  /// number of header lines to skip in the file
  unsigned int _lines_to_skip;

  /// dislocation densities
  std::vector<Real> _densities;
};
