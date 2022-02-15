//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GriddedData.h"
#include "MooseUtils.h"
#include <fstream>
#include <algorithm>

/**
 * Creates a GriddedData object by reading info from file_name
 * A grid is defined in _grid.
 *   For example, if grid[0] = {1, 2, 3} and grid[1] = {-1, 1}
 *   this defines a 2D grid (_dim = 2), with points
 *   (1,-1), (2,-1), (3,-1), (1,1), (2,1), (3,1)
 * The i_th axis of the grid corresponds to the axes[i] axis
 *   of the simulation: see the function getAxes
 * Values at each grid point are stored in _fcn.
 *   They are ordered as in the example above.
 * _step is just a quantity used in evaluateFcn
 * The file must have the following format:
 *   All blank lines and lines starting with # are ignored
 *   The first significant line (not blank or starting with #)
 *   should be either
 *     AXIS X
 *   or
 *     AXIS Y
 *   or
 *     AXIS Z
 *   or
 *     AXIS T
 *   The next significant line should be a space-separated
 *   array of real numbers defining the grid along that axis
 *   direction.
 *   Any number of AXIS and subsequent space-separated arrays
 *   can be defined, but if using this in conjunction with
 *   PiecewiseMultilinear, a maximum of 4 should be defined.
 *   The AXIS lines define the grid in the MOOSE simulation reference
 *   frame, and is used by PiecewiseMultilinear, for instance.
 *   The next significant line should be DATA
 *   All significant lines after DATA should be the function values
 *   at each grid point, on any number of lines of the file, but
 *   each line must be space separated.  The ordering is such
 *   that when the function is evaluated, f[i,j,k,l] corresponds
 *   to the i + j*Ni + k*Ni*Nj + l*Ni*Nj*Nk data value.  Here
 *   i>=0 corresponds to the index along the first AXIS, and Ni is
 *   the number of grid points along that axis, etc.
 *   See the function parse for an example.
 */
GriddedData::GriddedData(std::string file_name)
{
  parse(_dim, _axes, _grid, _fcn, _step, file_name);
}

unsigned int
GriddedData::getDim()
{
  return _dim;
}

void
GriddedData::getAxes(std::vector<int> & axes)
{
  axes.resize(_dim);
  std::copy(_axes.begin(), _axes.end(), axes.begin());
}

void
GriddedData::getGrid(std::vector<std::vector<Real>> & grid)
{
  grid.resize(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
  {
    grid[i].resize(_grid[i].size());
    std::copy(_grid[i].begin(), _grid[i].end(), grid[i].begin());
  }
}

void
GriddedData::getFcn(std::vector<Real> & fcn)
{
  fcn.resize(_fcn.size());
  std::copy(_fcn.begin(), _fcn.end(), fcn.begin());
}

Real
GriddedData::evaluateFcn(const GridIndex & ijk)
{
  if (ijk.size() != _dim)
    mooseError(
        "Gridded data evaluateFcn called with ", ijk.size(), " arguments, but expected ", _dim);
  unsigned int index = ijk[0];
  for (unsigned int i = 1; i < _dim; ++i)
    index += ijk[i] * _step[i];
  if (index >= _fcn.size())
    mooseError("Gridded data evaluateFcn attempted to access index ",
               index,
               " of function, but it contains only ",
               _fcn.size(),
               " entries");
  return _fcn[index];
}

void
GriddedData::parse(unsigned int & dim,
                   std::vector<int> & axes,
                   std::vector<std::vector<Real>> & grid,
                   std::vector<Real> & f,
                   std::vector<unsigned int> & step,
                   std::string file_name)
{
  // initialize
  dim = 0;
  axes.resize(0);
  grid.resize(0);
  f.resize(0);

  // open file and initialize quantities
  std::ifstream file(file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + file_name + "' from GriddedData.");
  std::string line;
  bool reading_grid_data = false;
  bool reading_value_data = false;

  // read file line-by-line extracting data
  while (getSignificantLine(file, line))
  {
    // look for AXIS keywords
    reading_grid_data = false;
    if (line.compare("AXIS X") == 0)
    {
      dim += 1;
      reading_grid_data = true;
      axes.push_back(0);
    }
    else if (line.compare("AXIS Y") == 0)
    {
      dim += 1;
      reading_grid_data = true;
      axes.push_back(1);
    }
    else if (line.compare("AXIS Z") == 0)
    {
      dim += 1;
      reading_grid_data = true;
      axes.push_back(2);
    }
    else if (line.compare("AXIS T") == 0)
    {
      dim += 1;
      reading_grid_data = true;
      axes.push_back(3);
    }

    // just found an AXIS keyword
    if (reading_grid_data)
    {
      grid.resize(dim); // add another dimension to the grid
      grid[dim - 1].resize(0);
      if (getSignificantLine(file, line))
        splitToRealVec(line, grid[dim - 1]);
      continue; // read next line from file
    }

    // previous significant line must have been DATA
    if (reading_value_data)
      splitToRealVec(line, f);

    // look for DATA keyword
    if (line.compare("DATA") == 0)
      reading_value_data = true;

    // ignore any other lines - if we get here probably the data file is corrupt
  }

  // check that some axes have been defined
  if (dim == 0)
    mooseError("No valid AXIS lines found by GriddedData");

  // step is useful in evaluateFcn
  step.resize(dim);
  step[0] = 1; // this is actually not used
  for (unsigned int i = 1; i < dim; ++i)
    step[i] = step[i - 1] * grid[i - 1].size();

  // perform some checks
  unsigned int num_data_points = 1;
  for (unsigned int i = 0; i < dim; ++i)
  {
    if (grid[i].size() == 0)
      mooseError("Axis ", i, " in your GriddedData has zero size");
    num_data_points *= grid[i].size();
  }
  if (num_data_points != f.size())
    mooseError("According to AXIS statements in GriddedData, number of data points is ",
               num_data_points,
               " but ",
               f.size(),
               " function values were read from file");
}

bool
GriddedData::getSignificantLine(std::ifstream & file_stream, std::string & line)
{
  while (std::getline(file_stream, line))
  {
    if (line.size() == 0) // empty line: read next line from file
      continue;
    if (line[0] == '#') // just a comment: read next line from file
      continue;
    // have got a significant line
    return true;
  }
  // have run out of file
  return false;
}

void
GriddedData::splitToRealVec(const std::string & input_string, std::vector<Real> & output_vec)
{
  std::vector<Real> values;
  bool status = MooseUtils::tokenizeAndConvert<Real>(input_string, values, " ");

  if (!status)
    mooseError("GriddedData: Failed to convert string into Real when reading line ", input_string);

  for (auto val : values)
    output_vec.push_back(val);
}
