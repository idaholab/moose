//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothCircleFromFileIC.h"

registerMooseObject("PhaseFieldApp", SmoothCircleFromFileIC);

InputParameters
SmoothCircleFromFileIC::validParams()
{
  InputParameters params = SmoothCircleBaseIC::validParams();
  params.addClassDescription("Multiple smooth circles read from a text file");
  params.addRequiredParam<FileName>("file_name", "File containing circle centers and radii");

  return params;
}

SmoothCircleFromFileIC::SmoothCircleFromFileIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters),
    _data(0),
    _file_name(getParam<FileName>("file_name")),
    _txt_reader(_file_name, &_communicator),
    _n_circles(0)
{
  // Read names and vectors from file
  _txt_reader.read();
  _col_names = _txt_reader.getNames();
  _data = _txt_reader.getData();
  _n_circles = _data[0].size();

  // Check that the file has all the correct information
  for (unsigned int i = 0; i < _col_names.size(); ++i)
  {
    // Check that columns have uniform lengths
    if (_data[i].size() != _n_circles)
      mooseError("Columns in ", _file_name, " do not have uniform lengths.");

    // Determine which columns correspond to which parameters.
    if (_col_names[i] == "x")
      _col_map[X] = i;
    else if (_col_names[i] == "y")
      _col_map[Y] = i;
    else if (_col_names[i] == "z")
      _col_map[Z] = i;
    else if (_col_names[i] == "r")
      _col_map[R] = i;
  }

  // Check that the required columns are present
  if (_col_map[X] == -1)
    mooseError("No column in ", _file_name, " labeled 'x'.");
  if (_col_map[Y] == -1)
    mooseError("No column in ", _file_name, " labeled 'y'.");
  if (_col_map[Z] == -1)
    mooseError("No column in ", _file_name, " labeled 'z'.");
  if (_col_map[R] == -1)
    mooseError("No column in ", _file_name, " labeled 'r'.");
}

void
SmoothCircleFromFileIC::computeCircleRadii()
{
  _radii.assign(_data[_col_map[R]].begin(), _data[_col_map[R]].end());
}

void
SmoothCircleFromFileIC::computeCircleCenters()
{
  _centers.resize(_n_circles);
  for (unsigned int i = 0; i < _n_circles; ++i)
  {
    _centers[i](0) = _data[_col_map[X]][i];
    _centers[i](1) = _data[_col_map[Y]][i];
    _centers[i](2) = _data[_col_map[Z]][i];
  }
}
