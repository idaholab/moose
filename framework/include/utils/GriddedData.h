//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseError.h"
#include "MooseUtils.h"

#include <iosfwd>
#include <string>
#include <vector>

/**
 * Container for holding a function defined on a grid of arbitrary dimension.
 *
 * Information is read from a file.  The file contains the grid, which
 * has dimension _dim, and consists of _dim vectors of Reals.  The
 * file also contains the function values at each grid point.  The
 * file also contains information on how to embed the grid into a
 * MOOSE simulation.  This is achieved through specifying the MOOSE
 * direction that each grid axis corresponds to.  For instance, the
 * first grid axis might correspond to the MOOSE "y" direction, the
 * second grid axis might correspond to the MOOSE "t" direction, etc.
 */
class GriddedData
{
public:
  /**
   * Construct with a file name
   */
  GriddedData(std::string file_name);

  virtual ~GriddedData() = default;

  typedef MooseUtils::SemidynamicVector<Real, 4> GridPoint;
  typedef MooseUtils::SemidynamicVector<ADReal, 4> ADGridPoint;
  typedef MooseUtils::SemidynamicVector<unsigned int, 4> GridIndex;

  /**
   * Returns the dimensionality of the grid.
   *
   * This may have nothing to do with the dimensionality of the
   * simulation.  Eg, a 2D grid with axes (Y,Z) (so dim=2) be used in
   * a 3D simulation.
   */
  unsigned int getDim();

  /**
   * Yields axes information.
   * If axes[i] == 0 then the i_th axis in the grid data corresponds to the x axis in the simulation
   * If axes[i] == 1 then the i_th axis in the grid data corresponds to the y axis in the simulation
   * If axes[i] == 2 then the i_th axis in the grid data corresponds to the z axis in the simulation
   * If axes[i] == 3 then the i_th axis in the grid data corresponds to the time in the simulation
   */
  void getAxes(std::vector<int> & axes);

  /**
   * Yields the grid.
   * grid[i] = a vector of Reals that define the i_th axis of the grid.
   */
  void getGrid(std::vector<std::vector<Real>> & grid);

  /**
   * Yields the values defined at the grid points.
   */
  void getFcn(std::vector<Real> & fcn);

  /**
   * Evaluates the function at a given grid point.
   * For instance, evaluateFcn({n,m}) = value at (grid[0][n], grid[1][m]), for a function defined on
   * a 2D grid
   */
  Real evaluateFcn(const GridIndex & ijk);

  /**
   * parse the file_name extracting information.
   * Here is an example file:
   * # this is a comment line at start of file
   * AXIS Y
   * -1.5 0
   * # there is no reason why the x axis can't be second
   * AXIS X
   * 1 2 3
   * # This example has a 3D grid, but the 3rd direction is time
   * AXIS T
   * 0 199
   * # now some data
   * DATA
   * # following for x=1, t=0
   * 1 2
   * # following for x=2, t=0
   * 2 3
   * # following for x=3, t=0
   * 2 3
   * # following for x=1, t=199
   * 89 900
   * # following for x=2, t=199, and x=3, t=199
   * 1 -3 -5 -6.898
   * # end of file
   */
  static void parse(unsigned int & dim,
                    std::vector<int> & axes,
                    std::vector<std::vector<Real>> & grid,
                    std::vector<Real> & f,
                    std::vector<unsigned int> & step,
                    std::string file_name);

private:
  unsigned int _dim;
  std::vector<int> _axes;
  std::vector<std::vector<Real>> _grid;
  std::vector<Real> _fcn;
  std::vector<unsigned int> _step;
  /**
   * Extracts the next line from file_stream that is:
   *  - not empty
   *  - does not start with #
   * Returns true if such a line was found,
   * otherwise returns false
   */
  static bool getSignificantLine(std::ifstream & file_stream, std::string & line);

  /**
   * Splits an input_string using space as the separator
   * Converts the resulting items to Real, and adds these
   * to the end of output_vec
   */
  static void splitToRealVec(const std::string & input_string, std::vector<Real> & output_vec);

  void updateGrid(unsigned int & dim, const std::vector<Real> & axis_i, const int axis_index);
};
