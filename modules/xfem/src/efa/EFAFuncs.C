//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAFuncs.h"

namespace Efa
{

double
linearQuadShape2D(unsigned int node_id, std::vector<double> & xi_2d)
{
  double node_xi[4][2] = {{-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}};
  return 0.25 * (1.0 + node_xi[node_id][0] * xi_2d[0]) * (1.0 + node_xi[node_id][1] * xi_2d[1]);
}

double
linearTriShape2D(unsigned int node_id, std::vector<double> & xi_2d)
{
  std::vector<double> area_xi(3, 0.0);
  area_xi[0] = xi_2d[0];
  area_xi[1] = xi_2d[1];
  area_xi[2] = 1.0 - xi_2d[0] - xi_2d[1];
  return area_xi[node_id];
}

double
linearHexShape3D(unsigned int node_id, std::vector<double> & xi_3d)
{
  double node_xi[8][3] = {{-1.0, -1.0, -1.0},
                          {1.0, -1.0, -1.0},
                          {1.0, 1.0, -1.0},
                          {-1.0, 1.0, -1.0},
                          {-1.0, -1.0, 1.0},
                          {1.0, -1.0, 1.0},
                          {1.0, 1.0, 1.0},
                          {-1.0, 1.0, 1.0}};
  return 0.125 * (1.0 + node_xi[node_id][0] * xi_3d[0]) * (1.0 + node_xi[node_id][1] * xi_3d[1]) *
         (1.0 + node_xi[node_id][2] * xi_3d[2]);
}

double
linearTetShape3D(unsigned int node_id, std::vector<double> & xi_3d)
{
  std::vector<double> vol_xi(4, 0.0);
  for (unsigned int i = 0; i < 3; ++i)
    vol_xi[i] = xi_3d[i];
  vol_xi[3] = 1.0 - xi_3d[0] - xi_3d[1] - xi_3d[2];
  return vol_xi[node_id];
}

} // namespace Efa
