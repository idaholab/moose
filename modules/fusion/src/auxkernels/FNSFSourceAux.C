//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FNSFSourceAux.h"
#include "FNSFUtils.h"

using namespace FNSF;

registerMooseObject("FusionApp", FNSFSourceAux);

std::pair<int, int>
FNSFSourceAux::index_xi_depth(Real xi,
                              Real depth,
                              const std::vector<Real> & inner_xi_grid,
                              const std::vector<Real> & outer_xi_grid,
                              const std::vector<Real> & depth_grid)
{
  // Perform a linear search to find the index of the given depth on the grid.
  // Compute the fractional location of the given value on the depth grid.
  int i_depth;
  Real frac;
  if (depth < depth_grid.front())
  {
    i_depth = 0;
    frac = 0.0;
  }
  else if (depth > depth_grid.back())
  {
    i_depth = depth_grid.size() - 2;
    frac = 1.0;
  }
  else
  {
    i_depth = 0;
    while (i_depth < depth_grid.size() - 2 && depth_grid[i_depth + 1] < depth)
      ++i_depth;
    frac = (depth - depth_grid.front()) / (depth_grid.back() - depth_grid.front());
  }

  // Perform a linear search to find the index of the given xi.  Use a linear
  // interpolation between the outer and inner grids (based on the depth).
  int i_xi;
  if (xi < outer_xi_grid.front())
  {
    i_xi = 0;
  }
  else if (xi > outer_xi_grid.back())
  {
    i_xi = outer_xi_grid.size() - 2;
  }
  else
  {
    i_xi = 0;
    while (i_xi < outer_xi_grid.size() - 2 &&
           frac * outer_xi_grid[i_xi + 1] + (1.0 - frac) * inner_xi_grid[i_xi + 1] < xi)
      ++i_xi;
  }

  return {i_xi, i_depth};
}

InputParameters
FNSFSourceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<std::vector<Real>>("inner_xi",
                                             "Grid of xi values (poloidal angle parameter) at the "
                                             "inner edge of the outboard blanket first wall.");
  params.addRequiredParam<std::vector<Real>>(
      "outer_xi",
      "Grid of xi values (poloidal angle parameter) at the outer edge of the outboard blanket.");
  params.addRequiredParam<std::vector<Real>>(
      "depth",
      "Grid of depth values (perpendicular distance from the last closed flux surface) for the "
      "outboard blanket.");
  params.addRequiredParam<std::vector<Real>>("source", "Source generation values for each cell.");
  return params;
}

FNSFSourceAux::FNSFSourceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _inner_xi_grid(getParam<std::vector<Real>>("inner_xi")),
    _outer_xi_grid(getParam<std::vector<Real>>("outer_xi")),
    _depth_grid(getParam<std::vector<Real>>("depth")),
    _source(getParam<std::vector<Real>>("source"))
{
}

Real
FNSFSourceAux::computeValue()
{
  const Point p = _q_point[_qp];
  Real r = std::sqrt(p(0) * p(0) + p(1) * p(1));
  Real z = p(2);

  Real xi, depth;
  std::tie(xi, depth) = find_xi_depth(r, z);

  int i_xi, i_depth;
  std::tie(i_xi, i_depth) = index_xi_depth(xi, depth, _inner_xi_grid, _outer_xi_grid, _depth_grid);

  int indx = i_depth * (_inner_xi_grid.size() - 1) + i_xi;
  return _source.at(indx);
}
