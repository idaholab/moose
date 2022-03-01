//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "GriddedData.h"

/**
 * Uses GriddedData to define data on a grid,
 * and does linear interpolation on that data to
 * provide function values.
 * Gridded data can be 1D, 2D, 3D or 4D.
 * See GriddedData for examples of file format.
 */
class PiecewiseMultiInterpolation : public Function
{
public:
  /**
   * Create new PiecewiseMultiInterpolation object.
   * This calls GriddedData to do most of the work
   */
  static InputParameters validParams();

  PiecewiseMultiInterpolation(const InputParameters & parameters);

  // Necessary for using forward declaration of GriddedData in std::unique_ptr
  virtual ~PiecewiseMultiInterpolation();

  using Function::value;
  /**
   * Given t and p, return the interpolated value.
   */
  virtual Real value(Real t, const Point & pt) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

protected:
  typedef GriddedData::GridPoint GridPoint;
  typedef GriddedData::ADGridPoint ADGridPoint;
  typedef GriddedData::GridIndex GridIndex;

  /// convert cartesian+time coordinates into grid coordinates
  template <bool is_ad>
  MooseADWrapper<GridPoint, is_ad> pointInGrid(const MooseADWrapper<Real, is_ad> & t,
                                               const MooseADWrapper<Point, is_ad> & p) const;

  /**
   * This does the core work.  Given a point, pt, defined
   * on the grid (not the MOOSE simulation reference frame),
   * interpolate the gridded data to this point
   */
  virtual Real sample(const GridPoint & pt) const = 0;
  virtual ADReal sample(const ADGridPoint & pt) const;

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
  std::vector<std::vector<Real>> _grid;

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
  void getNeighborIndices(std::vector<Real> in_arr,
                          Real x,
                          unsigned int & lower_x,
                          unsigned int & upper_x) const;
};
