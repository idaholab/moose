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

// Forward declarations
class GriddedData;

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
  PiecewiseMultilinear(const InputParameters & parameters);

  // Necessary for using forward declaration of GriddedData in std::unique_ptr
  virtual ~PiecewiseMultilinear();

  /**
   * Given t and p, return the interpolated value.
   */
  virtual Real value(Real t, const Point & pt) override;

private:

  /// object to provide function evaluations at points on the grid
  std::unique_ptr<GriddedData> _gridded_data;
  /// dimension of the grid
  unsigned int _dim;

  /**
   * _axes specifies how to embed the grid into the MOOSE coordinate frame
   * if _axes[i] = 0 then the i_th axes of the grid lies along the MOOSE x direction
   * if _axes[i] = 1 then the i_th axes of the grid lies along the MOOSE y direction
   * if _axes[i] = 2 then the i_th axes of the grid lies along the MOOSE z direction
   * if _axes[i] = 3 then the i_th axes of the grid lies along the MOOSE time direction
   */
  std::vector<int> _axes;

  /// the grid
  std::vector<std::vector<Real> > _grid;

  /**
   * This does the core work.  Given a point, pt, defined
   * on the grid (not the MOOSE simulation reference frame),
   * interpolate the gridded data to this point
   */
  Real sample(const std::vector<Real> & pt);

  /**
   * Operates on monotonically increasing in_arr.
   * Finds lower_x and upper_x which satisfy in_arr[lower_x] < x <= in_arr[upper_x].
   * End conditions: if x<in_arr[0] then lower_x = 0 = upper_x is returned
   *                 if x>in_arr[N-1] then lower_x = N-1 = upper_x is returned (N=size of in_arr)
   *
   * @param in_arr The monotonically increasing vector of real numbers
   * @param x The real value for which we want the neighbor indices
   * @param lower_x Upon return will contain lower_x specified above
   * @param upper_x Upon return will contain upper_x specified above
   */
  void getNeighborIndices(std::vector<Real> in_arr, Real x, unsigned int & lower_x, unsigned int & upper_x);
};

#endif //PIECEWISEMULTILINEAR_H
