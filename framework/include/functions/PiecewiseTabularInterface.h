//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "InputParameters.h"

class JSONFileReader;

class PiecewiseTabularInterface
{
public:
  static InputParameters validParams();

  PiecewiseTabularInterface(const MooseObject & object,
                            std::vector<Real> & data_x,
                            std::vector<Real> & data_y);

protected:
  /// Returns whether the raw data has been loaded already
  bool isRawDataLoaded() const { return _raw_data_loaded; }

  /// Reads data from supplied CSV file.
  void buildFromFile(const libMesh::Parallel::Communicator & comm);

  /// Reads data from supplied JSON reader.
  void buildFromJSON(const JSONFileReader & json_uo);

  /// Builds data from 'x' and 'y' parameters.
  void buildFromXandY();

  /// Builds data from 'xy_data' parameter.
  void buildFromXY();

  ///@{ if _has_axis is true point component to use as function argument, otherwise use t
  unsigned int _axis;
  const bool _has_axis;
  ///@}

private:
  /// The object
  const MooseObject & _object;

  /// Parameters supplied to the object
  const InputParameters & _parameters;

  /// Boolean to keep track of whether the data has been loaded
  bool _raw_data_loaded = false;

  ///@{ raw function data as read
  std::vector<Real> & _data_x;
  std::vector<Real> & _data_y;
  ///@}
};
