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

template <>
InputParameters
validParams<PiecewiseMultilinear>()
{
  InputParameters params = validParams<PiecewiseMultiInterpolation>();
  params.addClassDescription(
      "PiecewiseMultilinear performs linear interpolation on 1D, 2D, 3D or 4D "
      "data.  The data_file specifies the axes directions and the function "
      "values.  If a point lies outside the data range, the appropriate end "
      "value is used.");
  return params;
}

PiecewiseMultilinear::PiecewiseMultilinear(const InputParameters & parameters)
  : PiecewiseMultiInterpolation(parameters)
{
}

Real
PiecewiseMultilinear::sample(const std::vector<Real> & pt)
{
  /*
   * left contains the indices of the point to the 'left', 'down', etc, of pt
   * right contains the indices of the point to the 'right', 'up', etc, of pt
   * Hence, left and right define the vertices of the hypercube containing pt
   */
  std::vector<unsigned int> left(_dim);
  std::vector<unsigned int> right(_dim);
  for (unsigned int i = 0; i < _dim; ++i)
    getNeighborIndices(_grid[i], pt[i], left[i], right[i]);

  /*
   * The following just loops through all the vertices of the
   * hypercube containing pt, evaluating the function at all
   * those vertices, and weighting the contributions to the
   * final result depending on the distance of pt from the vertex
   */
  Real f = 0;
  Real weight;
  std::vector<unsigned int> arg(_dim);
  for (unsigned int i = 0; i < std::pow(2.0, int(_dim));
       ++i) // number of points in hypercube = 2^_dim
  {
    weight = 1;
    for (unsigned int j = 0; j < _dim; ++j)
      if ((i >> j) % 2 ==
          0) // shift i j-bits to the right and see if the result has a 0 as its right-most bit
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
    f += _gridded_data->evaluateFcn(arg) * weight;
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

  return f / weight;
}
