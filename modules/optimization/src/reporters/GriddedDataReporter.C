//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GriddedDataReporter.h"
#include "DelimitedFileReader.h"
#include "GriddedData.h"

registerMooseObject("isopodApp", GriddedDataReporter);

InputParameters
GriddedDataReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter to hold griddedData parameter values and a grid containing "
                             "their spatial and temporal coordinates");
  params.addRequiredParam<FileName>(
      "data_file",
      "File holding data for use with PiecewiseMultiInterpolation. Format: any empty line and any "
      "line "
      "beginning with # are ignored, all other lines are assumed to contain relevant information. "
      "The file must begin with specification of the grid. This is done through lines containing "
      "the keywords: AXIS X; AXIS Y; AXIS Z; or AXIS T. Immediately following the keyword line "
      "must be a space-separated line of real numbers which define the grid along the specified "
      "axis. These data must be monotonically increasing. After all the axes and their grids "
      "have been specified, there must be a line that is DATA. Following that line, function "
      "values are given in the correct order (they may be on individual lines, or be "
      "space-separated on a number of lines). When the function is evaluated, f[i,j,k,l] "
      "corresponds to the i + jNi + kNiNj + lNiNjNk data value. Here i>=0 corresponding to "
      "the index along the first AXIS, j>=0 corresponding to the index along the second AXIS, etc, "
      "and Ni = number of grid points along the first AXIS, etc.");
  return params;
}

GriddedDataReporter::GriddedDataReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _file_name(getParam<FileName>("data_file")),
    _parameters(declareValueByName<std::vector<Real>>("parameter", REPORTER_MODE_REPLICATED)),
    _grid(declareValueByName<std::vector<std::vector<Real>>>("grid", REPORTER_MODE_REPLICATED)),
    _axes(declareValueByName<std::vector<int>>("axes", REPORTER_MODE_REPLICATED)),
    _step(declareValueByName<std::vector<unsigned int>>("step", REPORTER_MODE_REPLICATED)),
    _dim(declareValueByName<unsigned int>("dim", REPORTER_MODE_REPLICATED))
{
  GriddedData::parse(_dim, _axes, _grid, _parameters, _step, _file_name);
}
