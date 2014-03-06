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

#ifndef GRIDDEDDATA_H
#define GRIDDEDDATA_H

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "MooseError.h"

/**
 * Container for holding a function defined on a grid of arbitrary dimension
 *
 * Information is read from a file.
 * The file contains the grid, which has dimension _dim, and consists
 *   of _dim vectors of Reals.
 * The file also contains the function values at each grid point.
 * The file also contains information on how to imbed the grid
 *   into a MOOSE simulation.  This is achieved through specifying the
 *   MOOSE direction that each grid axis corresponds to.  For instance,
 *   the first grid axis might correspond to the MOOSE "y" direction,
 *   the second grid axis might correspond to the MOOSE "t" direction, etc.
 */
class GriddedData
{
public:

  /**
   * Construct with a file name
   */
  GriddedData(std::string file_name);

  virtual ~GriddedData()
    {}

  /**
   * Returns the dimensionality of the grid.
   * This may have nothing to do with the dimensionality of
   * the simulation.  Eg, a 2D grid with axes (Y,Z) (so dim=2)
   * be used in a 3D simulation
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
   * grid[i] = a vector of Reals that define the i_th axis of the grid
   */
  void getGrid(std::vector<std::vector<Real> > & grid);

  /**
   * Yields the values defined at the grid points
   */
  void getFcn(std::vector<Real> & fcn);

  /**
   * Evaluates the function at a given grid point
   * for instance evaluateFcn({n,m}) = value at (grid[0][n], grid[1][m]), for a function defined on a 2D grid
   */
  Real evaluateFcn(const std::vector<unsigned int> & ijk);


private:

  unsigned int _dim;
  std::vector<int> _axes;
  std::vector<std::vector<Real> > _grid;
  std::vector<Real> _fcn;
  std::vector<unsigned int> _step;

  void parse(unsigned int & dim, std::vector<int> & axes, std::vector<std::vector<Real> > & grid, std::vector<Real> & f, std::vector<unsigned int> & step, std::string file_name);
  bool getSignificantLine(std::ifstream & file_stream, std::string & line);
  void splitToRealVec(const std::string & input_string, std::vector<Real> & output_vec);
};

#endif //GRIDDEDDATA_H




