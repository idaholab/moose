//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 * This reporter contains the data produced by the GriddedData object.  Because this data is in
 * a reporter, the data contained in it can be manipulated.
 **/

class GriddedDataReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  GriddedDataReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /// file containing griddedData
  FileName _file_name;
  /// value at each xyzt point
  std::vector<Real> & _parameters;
  /// contains xyzt coordinate values
  std::vector<std::vector<Real>> & _grid;
  std::vector<int> & _axes;
  /// step is stride length of each grid dimension
  std::vector<unsigned int> & _step;
  /// overall dimension of gridded data i.e. xyz is _dim=2
  unsigned int & _dim;
};
