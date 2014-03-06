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


#include "PiecewiseMultilinear.h"

/**
 * Uses GriddedData to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
template<>
InputParameters validParams<PiecewiseMultilinear>()
{
  InputParameters params = validParams<Function>();
  params.addParam<std::string>("data_file", "File holding data for use with PiecewiseMultilinear.  Format: any empty line and any line beginning with # are ignored, all other lines are assumed to contain relevant information.  The file must begin with specification of the grid.  This is done through lines containing the keywords: AXIS X; AXIS Y; AXIS Z; or AXIS T.  Immediately following the keyword line must be a space-separated line of real numbers which define the grid along the specified axis.  These data must be monotonically increasing.  After all the axes and their grids have been specified, there must be a line that is DATA.  Following that line, function values are given in the correct order (they may be on indivicual lines, or be space-separated on a number of lines).  When the function is evaluated, f[i,j,k,l] corresponds to the i + j*Ni + k*Ni*Nj + l*Ni*Nj*Nk data value.  Here i>=0 corresponding to the index along the first AXIS, j>=0 corresponding to the index along the second AXIS, etc, and Ni = number of grid points along the first AXIS, etc.");
  params.addClassDescription("PiecewiseMultilinear performs interpolation on 1D, 2D, 3D or 4D data.  The data_file specifies the axes directions and the function values.  If a point lies outside the data range, the appropriate end value is used.");
  return params;
}

/**
 * Most of the work here is done by GriddedData.
 * We just extract our own private variables,
 * and check for any incompatibilities in the data
 */
PiecewiseMultilinear::PiecewiseMultilinear(const std::string & name, InputParameters parameters) :
  Function(name, parameters)
{
  _gridded_data = new GriddedData(getParam<std::string>("data_file"));
  _dim = _gridded_data->getDim();
  _gridded_data->getAxes(_axes);
  _gridded_data->getGrid(_grid);

  // GriddedData does not require monotonicity of axes, but we do
  for (unsigned int i = 0; i < _dim; ++i)
    for (unsigned int j = 1; j < _grid[i].size(); ++j)
      if (_grid[i][j-1] >= _grid[i][j])
	mooseError("PiecewiseMultilinear needs monotonically-increasing axis data.  Axis " << i << " contains non-monotinicity at value " << _grid[i][j] << "\n");

  // GriddedData does not demand that each axis is independent, but we do
  std::set<int> s(_axes.begin(), _axes.end());
  if (s.size() != _dim)
    mooseError("PiecewiseMultilinear needs the AXES to be independent.  Check the AXES lines in your data file.\n");

}


PiecewiseMultilinear::~PiecewiseMultilinear()
{
  delete _gridded_data;
}


/**
 * Given t and p, return the interpolated value.
 * Note that t and p will be defined in the MOOSE simulation
 * reference frame, and using the _axes definitions found
 * in the data file (and extracted by GriddedData), this
 * is converted to a point in the grid reference frame.
 */
Real
PiecewiseMultilinear::value( Real t, const Point & p)
{
  // convert the inputs to an input to the sample function using _axes
  std::vector<Real> pt_in_grid(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
    {
      if (_axes[i] < 3)
	pt_in_grid[i] = p(_axes[i]);
      else if (_axes[i] == 3) // the time direction
	pt_in_grid[i] = t;
    }
  return sample(pt_in_grid);
}



/**
 * This does the core work.  Given a point, pt, defined
 * on the grid (not the MOOSE simulation reference frame),
 * interpolate the gridded data to this point
 */
Real
PiecewiseMultilinear::sample(const std::vector<Real> & pt)
{
  /*
   * left contains the indices of the point to the 'left', 'down', etc, of pt
   * right contains the indices of the point to the 'right', 'up', etc, of pt
   * Hence, left and right define the vertices of the hypercube containint pt
   */
  std::vector<unsigned int> left(_dim);
  std::vector<unsigned int> right(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
    {
      getNeighborIndices(_grid[i], pt[i], left[i], right[i]);
    }

  /*
   * The following just loops through all the vertices of the
   * hypercube containing pt, evaluating the function at all
   * those vertices, and weighting the contributions to the
   * final result depending on the distance of pt from the vertex
   */
  Real f = 0;
  Real weight;
  std::vector<unsigned int> arg(_dim);
  for (unsigned int i = 0; i < std::pow(2, _dim); ++i)
    {
      weight = 1;
      for (unsigned int j = 0; j < _dim; ++j)
	if ( (i >> j) % 2 == 0) // shift i j-bits to the right and see if the result has a 0 as its right-most bit
	  {
	    arg[j] = left[j];
	    if (left[j] != right[j])
	      weight *= std::abs(pt[j] - _grid[j][right[j]]);
	    else // unusual "end condition" case.  weight by 0.5 because we will encounter this twice
	      weight *= 0.5;
	  }
	else
	  {
	    arg[j] = right[j];
	    if (left[j] != right[j])
	      weight *= std::abs(pt[j] - _grid[j][left[j]]);
	    else // unusual "end condition" case.  weight by 0.5 because we will encounter this twice
	      weight *= 0.5;
	  }
      f += _gridded_data->evaluateFcn(arg)*weight;
    }

  /*
   * finally divide by the volume of the hypercube
   */
  weight = 1;
  for (unsigned int dim = 0; dim < pt.size(); ++dim)
    if (left[dim] != right[dim])
      weight *= _grid[dim][right[dim]] - _grid[dim][left[dim]];
    else // unusual "end condition" case.  weight by 1 to cancel the two 0.5 encountered previously
      weight *= 1;

  return f/weight;
}



/**
 * Operates on monotonically increasing inArr.
 * Finds lowerX and upperX which satisfy inArr[lowerX] < x <= inArr[upperX].
 * End conditions: if x<inArr[0] then lowerX = 0 = upperX is returned
 *                 if x>inArr[N-1] then lowerX = N-1 = upperX is returned (N=size of inArr)
 *
 * @param inArr The monotonically increasing vector of real numbers
 * @param x The real value for which we want the neighbour indices
 * @param lowerX Upon return will contain lowerX specified above
 * @param upperX Upon return will contain upperX specified above
 */
void
PiecewiseMultilinear::getNeighborIndices(std::vector<Real> inArr, Real x, unsigned int & lowerX, unsigned int & upperX )
{
  int N = inArr.size();
  if (x <= inArr[0])
  {
    lowerX = 0;
    upperX = 0;
  }
  else if (x >= inArr[N-1] )
  {
    lowerX = N-1;
    upperX = N-1;
  }
  else
  {
    std::vector<double>::iterator up = std::lower_bound(inArr.begin(), inArr.end(), x); // returns up which points at the first element in inArr that is not less than x
    upperX = std::distance(inArr.begin(), up);
    if (inArr[upperX] == x)
      lowerX = upperX;
    else
      lowerX = upperX - 1;
  }
}

