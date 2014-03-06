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

#ifndef PIECEWISEMULTILINEAR_H
#define PIECEWISEMULTILINEAR_H

#include "Function.h"
#include "GriddedData.h"


/**
 * Uses GriddedData to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
class PiecewiseMultilinear;

template<>
InputParameters validParams<PiecewiseMultilinear>();

class PiecewiseMultilinear : public Function
{
public:

  /**
   * Create new PiecewiseMultilinear object.
   * This calls GriddedData to do most of the work
   */
  PiecewiseMultilinear(const std::string & name, InputParameters parameters);
  virtual ~PiecewiseMultilinear();

  /**
   * Given t and p, return the interpolated value.
   */
  virtual Real value(Real t, const Point & pt);

private:

  GriddedData * _gridded_data; /// object to provide function evaluations at points on the grid
  unsigned int _dim;  /// dimension of the grid

  /*
   * _axes specifies how to embed the grid into the MOOSE coordinate frame
   * if _axes[i] = 0 then the i_th axes of the grid lies along the MOOSE x direction
   * if _axes[i] = 1 then the i_th axes of the grid lies along the MOOSE y direction
   * if _axes[i] = 2 then the i_th axes of the grid lies along the MOOSE z direction
   * if _axes[i] = 3 then the i_th axes of the grid lies along the MOOSE time direction
   */
  std::vector<int> _axes;

  std::vector<std::vector<Real> > _grid; // the grid

  /**
   * This does the core work.  Given a point, pt, defined
   * on the grid (not the MOOSE simulation reference frame),
   * interpolate the gridded data to this point
   */
  Real sample(const std::vector<Real> & pt);

  /**
   * Operates on monotonically increasing inArr.
   * Finds lowerX and upperX which satisfy inArr[lowerX] < x <= inArr[upperX].
   * End conditions: if x<inArr[0] then lowerX = 0 = upperX is returned
   *                 if x>inArr[N-1] then lowerX = N-1 = upperX is returned (N=size of inArr)
   */
  void getNeighborIndices(std::vector<Real> inArr, Real x, unsigned int & lowerX, unsigned int & upperX );
};

#endif //PIECEWISEMULTILINEAR_H




