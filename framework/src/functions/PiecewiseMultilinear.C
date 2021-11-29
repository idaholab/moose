//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseMultilinear.h"
#include "GriddedData.h"

registerMooseObject("MooseApp", PiecewiseMultilinear);

InputParameters
PiecewiseMultilinear::validParams()
{
  InputParameters params = PiecewiseMultiInterpolation::validParams();
  params.addClassDescription(
      "PiecewiseMultilinear performs linear interpolation on 1D, 2D, 3D or 4D "
      "data.  The data_file specifies the axes directions and the function "
      "values.  If a point lies outside the data range, the appropriate end "
      "value is used.");
  params.addParam<Real>(
      "epsilon", 1e-12, "Finite differencing parameter for gradient and time derivative");
  return params;
}

PiecewiseMultilinear::PiecewiseMultilinear(const InputParameters & parameters)
  : PiecewiseMultiInterpolation(parameters), _epsilon(getParam<Real>("epsilon"))
{
}

Real
PiecewiseMultilinear::sample(const GridPoint & pt) const
{
  return sampleInternal<false>(pt);
}

ADReal
PiecewiseMultilinear::sample(const ADGridPoint & pt) const
{
  return sampleInternal<true>(pt);
}

template <bool is_ad>
MooseADWrapper<Real, is_ad>
PiecewiseMultilinear::sampleInternal(const MooseADWrapper<GridPoint, is_ad> pt) const
{
  /*
   * left contains the indices of the point to the 'left', 'down', etc, of pt
   * right contains the indices of the point to the 'right', 'up', etc, of pt
   * Hence, left and right define the vertices of the hypercube containing pt
   */
  GridIndex left(_dim);
  GridIndex right(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
    getNeighborIndices(_grid[i], MetaPhysicL::raw_value(pt[i]), left[i], right[i]);

  /*
   * The following just loops through all the vertices of the
   * hypercube containing pt, evaluating the function at all
   * those vertices, and weighting the contributions to the
   * final result depending on the distance of pt from the vertex
   */
  MooseADWrapper<Real, is_ad> f = 0;
  MooseADWrapper<Real, is_ad> weight;
  GridIndex arg(_dim);
  // number of points in hypercube = 2^_dim
  for (unsigned int i = 0; i < (1u << _dim); ++i)
  {
    weight = 1;
    for (unsigned int j = 0; j < _dim; ++j)
      // shift i j-bits to the right and see if the result has a 0 as its right-most bit
      if ((i >> j) % 2 == 0)
      {
        arg[j] = left[j];
        if (left[j] != right[j])
          weight *= std::abs(pt[j] - _grid[j][right[j]]);
        else
          // unusual "end condition" case. weight by 0.5 because we will encounter this twice
          weight *= 0.5;
      }
      else
      {
        arg[j] = right[j];
        if (left[j] != right[j])
          weight *= std::abs(pt[j] - _grid[j][left[j]]);
        else
          // unusual "end condition" case. weight by 0.5 because we will encounter this twice
          weight *= 0.5;
      }
    f += _gridded_data->evaluateFcn(arg) * weight;
  }

  /*
   * finally divide by the volume of the hypercube
   */
  weight = 1;
  for (unsigned int dim = 0; dim < pt.size(); ++dim)
    if (left[dim] != right[dim])
      weight *= _grid[dim][right[dim]] - _grid[dim][left[dim]];
    else
      // unusual "end condition" case. weight by 1 to cancel the two 0.5 encountered previously
      weight *= 1;

  return f / weight;
}

RealGradient
PiecewiseMultilinear::gradient(Real t, const Point & p) const
{
  RealGradient grad;

  // sample center point
  auto s1 = sample(pointInGrid<false>(t, p));

  // sample epsilon steps in all directions
  for (const auto dir : _axes)
    if (dir < 3)
    {
      Point pp = p;
      pp(dir) += _epsilon;
      grad(dir) = (sample(pointInGrid<false>(t, pp)) - s1) / _epsilon;
    }

  return grad;
}

Real
PiecewiseMultilinear::timeDerivative(Real t, const Point & p) const
{
  return (sample(pointInGrid<false>(t + _epsilon, p)) - sample(pointInGrid<false>(t, p))) /
         _epsilon;
}
