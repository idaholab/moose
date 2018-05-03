//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshBasePD.h"

#include "libmesh/utility.h"

template <>
InputParameters
validParams<MeshBasePD>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addClassDescription("Base class for generating mesh for peridynamic simulation");

  params.addParam<Real>("horizon_radius", "Value of horizon size in terms of radius");
  params.addParam<Real>("horizon_number",
                        "The material points spacing number, i.e. ratio of horizon radius to the "
                        "effective mesh spacing");
  params.addParam<std::vector<Point>>("cracks_start",
                                      "Cartesian coordinates where predefined line cracks start");
  params.addParam<std::vector<Point>>("cracks_end",
                                      "Cartesian coordinates where predefined line cracks end");
  params.addParam<std::vector<Real>>("cracks_width", "Widths of predefined line cracks");

  return params;
}

MeshBasePD::MeshBasePD(const InputParameters & parameters)
  : MooseMesh(parameters),
    _horizon_radius(isParamValid("horizon_radius") ? getParam<Real>("horizon_radius") : 0),
    _horizon_number(isParamValid("horizon_number") ? getParam<Real>("horizon_number") : 0),
    _has_cracks(isParamValid("cracks_start") || isParamValid("cracks_end"))
{
  if (isParamValid("horizon_radius") && isParamValid("horizon_number"))
    mooseError(
        "You can specify only one option: horizon_radius or horizon_number in the mesh block!");

  if (!isParamValid("horizon_radius") && !isParamValid("horizon_number"))
    mooseError("You must specify one option: horizon_radius or horizon_number in the mesh block!");

  if (_has_cracks)
  {
    _cracks_start = getParam<std::vector<Point>>("cracks_start");
    _cracks_end = getParam<std::vector<Point>>("cracks_end");
  }
  else
  {
    _cracks_start.push_back(Point(0, 0, 0));
    _cracks_end.push_back(Point(0, 0, 0));
  }

  if (_cracks_start.size() != _cracks_end.size())
    mooseError("Number of cracks start points is NOT the same as number of cracks end points!");

  if (_has_cracks)
  {
    if (isParamValid("cracks_width"))
    {
      _cracks_width = getParam<std::vector<Real>>("cracks_width");
      if (_cracks_width.size() != _cracks_start.size())
        mooseError("Number of cracks width is NOT the same as number of cracks!");
    }
    else
    {
      for (unsigned int i = 0; i < _cracks_start.size(); ++i)
        _cracks_width.push_back(0);
    }
  }
  else
    _cracks_width.push_back(0);
}

unsigned int
MeshBasePD::dimension() const
{
  return _dim;
}

dof_id_type
MeshBasePD::nNodes() const
{
  return _total_nodes;
}

dof_id_type
MeshBasePD::nElem() const
{
  return _total_bonds;
}

Real
MeshBasePD::computeHorizon(Real spacing)
{
  if (isParamValid("horizon_number"))
    return _horizon_number * spacing;
  else
    return _horizon_radius;
}

void
MeshBasePD::findNodeNeighbor()
{
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    Real dis = 0.0;
    for (unsigned int j = 0; j < _total_nodes; ++j)
    {
      dis = (_pdnode[i].coord - _pdnode[j].coord).norm();
      if (_pdnode[i].blockID == _pdnode[j].blockID && dis <= 1.0001 * _pdnode[i].horizon && j != i)
      {
        // check whether pdnode i falls in the region whose bonds may need to be removed due to the
        // pre-existing cracks
        bool intersect = false;
        for (unsigned int k = 0; k < _cracks_start.size(); ++k)
        {
          if (checkInside(_cracks_start[k],
                          _cracks_end[k],
                          _pdnode[i].coord,
                          _cracks_width[k] + 4.0 * _pdnode[i].horizon,
                          4.0 * _pdnode[i].horizon))
            intersect = intersect || checkCrackIntersection(_cracks_start[k],
                                                            _cracks_end[k],
                                                            _pdnode[i].coord,
                                                            _pdnode[j].coord,
                                                            _cracks_width[k]);
        }
        // remove bonds cross the crack to form crack surface
        if (!intersect)
        {
          // Use the addition balance scheme to remove unbalanced interactions
          // check whether j was already considered as a neighbor of i, if not, add j to i's
          // neighborlist
          if (std::find(_node_neighbors[i].begin(), _node_neighbors[i].end(), j) ==
              _node_neighbors[i].end())
          {
            _node_neighbors[i].push_back(j);
            _pdnode[i].volumesum += _pdnode[j].volume;
          }
          // check whether i was also considered as a neighbor of j, if not, add i to j's
          // neighborlist
          if (std::find(_node_neighbors[j].begin(), _node_neighbors[j].end(), i) ==
              _node_neighbors[j].end())
          {
            _node_neighbors[j].push_back(i);
            _pdnode[j].volumesum += _pdnode[i].volume;
          }
        }
      }

      // the n nearest neighbors
      if (_pdnode[i].blockID == _pdnode[j].blockID && dis <= 1.5 * _pdnode[i].horizon && j != i)
        _node_n_nearest_neighbors[i].push_back(j);
    }
  }
}

void
MeshBasePD::setupDGNodeInfo()
{
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    std::vector<dof_id_type> neighbors = _node_neighbors[i];
    _dg_nodeinfo[i].resize(neighbors.size());
    _dg_bond_volumesum[i].resize(neighbors.size());
    _dg_node_volumesum[i] = 0.0;
    for (unsigned int j = 0; j < neighbors.size(); ++j)
    {
      _dg_bond_volumesum[i][j] = 0.0;
      // add the particle pair itself
      _dg_nodeinfo[i][j].push_back(j);
      std::vector<dof_id_type> j_n_nearest_neighbors = _node_n_nearest_neighbors[neighbors[j]];
      for (unsigned int k = 0; k < j_n_nearest_neighbors.size(); ++k)
      {
        auto it = std::find(neighbors.begin(), neighbors.end(), j_n_nearest_neighbors[k]);
        if (it != neighbors.end() && i != j_n_nearest_neighbors[k])
        {
          // only save the corresponding index in neighbor list, rather than the node id
          unsigned int pos = it - neighbors.begin();
          _dg_nodeinfo[i][j].push_back(pos);
          _dg_bond_volumesum[i][j] += _pdnode[j_n_nearest_neighbors[k]].volume;
          _dg_node_volumesum[i] += _pdnode[j_n_nearest_neighbors[k]].volume;
        }
      }
    }
  }
}

std::vector<dof_id_type>
MeshBasePD::neighbors(dof_id_type node_id)
{
  return _node_neighbors[node_id];
}

unsigned int
MeshBasePD::neighborID(dof_id_type node_i, dof_id_type node_j)
{
  std::vector<dof_id_type> neighbors = _node_neighbors[node_i];
  auto it = std::find(neighbors.begin(), neighbors.end(), node_j);
  if (it != neighbors.end())
    return it - neighbors.begin();
  else
    mooseError(
        "Material point ", node_j, " is not in the neighbor list of material point ", node_i);

  return -1;
}

std::vector<dof_id_type>
MeshBasePD::bonds(dof_id_type node_id)
{
  return _node_bonds[node_id];
}

std::vector<unsigned int>
MeshBasePD::dgNodeInfo(dof_id_type node_id, unsigned int neighbor_id)
{
  return _dg_nodeinfo[node_id][neighbor_id];
}

Point
MeshBasePD::coord(dof_id_type node_id)
{
  return _pdnode[node_id].coord;
}

Real
MeshBasePD::volume(dof_id_type node_id)
{
  return _pdnode[node_id].volume;
}

Real
MeshBasePD::volumeSum(dof_id_type node_id)
{
  return _pdnode[node_id].volumesum;
}

Real
MeshBasePD::dgBondVolumeSum(dof_id_type node_id, unsigned int neighbor_id)
{
  return _dg_bond_volumesum[node_id][neighbor_id];
}

Real
MeshBasePD::dgNodeVolumeSum(dof_id_type node_id)
{
  return _dg_node_volumesum[node_id];
}

unsigned int
MeshBasePD::nneighbors(dof_id_type node_id)
{
  return _node_neighbors[node_id].size();
}

Real
MeshBasePD::mesh_spacing(dof_id_type node_id)
{
  return _pdnode[node_id].mesh_spacing;
}

Real
MeshBasePD::horizon(dof_id_type node_id)
{
  return _pdnode[node_id].horizon;
}

bool
MeshBasePD::checkInside(Point crack_start, Point crack_end, Point point, Real crack_width, Real tol)
{
  Real crack_length = (crack_end - crack_start).norm();
  bool inside = crack_length;

  if (inside)
  {
    Real a = crack_end(1) - crack_start(1);
    Real b = crack_start(0) - crack_end(0);
    Real c = crack_end(0) * crack_start(1) - crack_end(1) * crack_start(0);
    inside *= std::abs(a * point(0) + b * point(1) + c) / crack_length < crack_width / 2.0;

    a = crack_end(0) - crack_start(0);
    b = crack_end(1) - crack_start(1);
    c = 0.5 * (crack_start(1) * crack_start(1) - crack_end(1) * crack_end(1) -
               crack_end(0) * crack_end(0) + crack_start(0) * crack_start(0));
    inside *= std::abs(a * point(0) + b * point(1) + c) / crack_length < (tol + crack_length) / 2.0;
  }

  return inside;
}

bool
MeshBasePD::checkCrackIntersection(Point A, Point B, Point C, Point D, Real width)
{
  bool intersect0 = false;
  bool intersect1 = false;
  bool intersect2 = false;
  if ((B - A).norm())
  {
    intersect0 = checkSegmentIntersection(A, B, C, D);
    if (width != 0.)
    {
      Real distAB = (A - B).norm();
      Real CosAB = (B(0) - A(0)) / distAB;
      Real SinAB = (B(1) - A(1)) / distAB;
      Real NewAX = A(0) - width / 2.0 * SinAB;
      Real NewAY = A(1) + width / 2.0 * CosAB;
      Real NewBX = B(0) - width / 2.0 * SinAB;
      Real NewBY = B(1) + width / 2.0 * CosAB;
      Point NewA = Point(NewAX, NewAY, 0.);
      Point NewB = Point(NewBX, NewBY, 0.);
      intersect1 = checkSegmentIntersection(NewA, NewB, C, D);
      NewAX = A(0) + width / 2.0 * SinAB;
      NewAY = A(1) - width / 2.0 * CosAB;
      NewBX = B(0) + width / 2.0 * SinAB;
      NewBY = B(1) - width / 2.0 * CosAB;
      NewA = Point(NewAX, NewAY, 0.);
      NewB = Point(NewBX, NewBY, 0.);
      intersect2 = checkSegmentIntersection(NewA, NewB, C, D);
    }
  }

  return intersect0 || intersect1 || intersect2;
}

bool
MeshBasePD::checkSegmentIntersection(Point A, Point B, Point C, Point D)
{
  // Fail if the segments share an end-point
  if ((A(0) == C(0) && A(1) == C(1)) || (B(0) == C(0) && B(1) == C(1)) ||
      (A(0) == D(0) && A(1) == D(1)) || (B(0) == D(0) && B(1) == D(1)))
  {
    return false;
  }

  // Fail if the segments intersect at a given end-point but not normal to the crack
  if ((A(1) - B(1)) / (A(0) - B(0)) == (A(1) - C(1)) / (A(0) - C(0)) ||
      (A(1) - B(1)) / (A(0) - B(0)) == (A(1) - D(1)) / (A(0) - D(0)) ||
      (C(1) - D(1)) / (C(0) - D(0)) == (C(1) - A(1)) / (C(0) - A(0)) ||
      (C(1) - D(1)) / (C(0) - D(0)) == (C(1) - B(1)) / (C(0) - B(0)))
  {
    Real cosAB_CD = (B - A) * (D - C) / ((B - A).norm() * (D - C).norm());
    if (cosAB_CD > -0.08715574 && cosAB_CD < 0.08715574)
      return false;
  }

  // Translate the system so that point A is on the origin
  B(0) -= A(0);
  B(1) -= A(1);
  C(0) -= A(0);
  C(1) -= A(1);
  D(0) -= A(0);
  D(1) -= A(1);

  // Length of segment A-B
  Real distAB = B.norm();

  // Rotate the system so that point B is on the positive X axis
  Real CosAB = B(0) / distAB;
  Real SinAB = B(1) / distAB;
  Real newX = C(0) * CosAB + C(1) * SinAB;
  C(1) = C(1) * CosAB - C(0) * SinAB;
  C(0) = newX;
  newX = D(0) * CosAB + D(1) * SinAB;
  D(1) = D(1) * CosAB - D(0) * SinAB;
  D(0) = newX;

  // Fail if segment C-D doesn't cross segment A-B
  if ((C(1) < 0. && D(1) < 0.) || (C(1) >= 0. && D(1) >= 0.))
    return false;

  // Fail if segment C-D crosses segment A-B outside of segment A-B
  Real ABpos = D(0) + (C(0) - D(0)) * D(1) / (D(1) - C(1));
  if (ABpos < 0. || ABpos > distAB)
    return false;

  return true;
}
