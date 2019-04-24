//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsMesh.h"

#include "libmesh/elem.h"

registerMooseObject("PeridynamicsApp", PeridynamicsMesh);

template <>
InputParameters
validParams<PeridynamicsMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  params.addClassDescription("Mesh class to store and return peridynamics specific mesh data");

  params.addParam<Real>("horizon_radius", "Value of horizon size in terms of radius");
  params.addParam<Real>("horizon_number",
                        "The material points spacing number, i.e. ratio of horizon radius to the "
                        "effective mesh spacing");
  params.addParam<Real>("ratio_of_horizons",
                        "Ratio of bond-associated horizon to nodal horizon. This is the only "
                        "parameters to control the size of bond-associated horizon.");
  params.addParam<std::vector<Point>>("cracks_start",
                                      "Cartesian coordinates where predefined line cracks start");
  params.addParam<std::vector<Point>>("cracks_end",
                                      "Cartesian coordinates where predefined line cracks end");
  params.addParam<std::vector<Real>>("cracks_width", "Widths of predefined line cracks");

  return params;
}

PeridynamicsMesh::PeridynamicsMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _horizon_radius(isParamValid("horizon_radius") ? getParam<Real>("horizon_radius") : 0),
    _has_horizon_number(isParamValid("horizon_number")),
    _horizon_number(_has_horizon_number ? getParam<Real>("horizon_number") : 0),
    _horizons_ratio(isParamValid("ratio_of_horizons") ? getParam<Real>("ratio_of_horizons") : 1.5),
    _has_cracks(isParamValid("cracks_start") || isParamValid("cracks_end")),
    _dim(declareRestartableData<unsigned int>("dim")),
    _total_nodes(declareRestartableData<unsigned int>("total_nodes")),
    _total_bonds(declareRestartableData<unsigned int>("total_bonds")),
    _node_mesh_spacing(declareRestartableData<std::vector<Real>>("node_mesh_spacing")),
    _node_horizon(declareRestartableData<std::vector<Real>>("node_horizon")),
    _node_vol(declareRestartableData<std::vector<Real>>("node_vol")),
    _node_horizon_vol(declareRestartableData<std::vector<Real>>("node_horizon_vol")),
    _node_blockID(declareRestartableData<std::vector<unsigned int>>("node_blockID")),
    _horizon_neighbors(
        declareRestartableData<std::vector<std::vector<dof_id_type>>>("horizon_neighbors")),
    _node_associated_bonds(
        declareRestartableData<std::vector<std::vector<dof_id_type>>>("node_associated_bonds")),
    _bah_dgneighbors(declareRestartableData<std::vector<std::vector<std::vector<unsigned int>>>>(
        "bond_associated_horizon_dgneighbors")),
    _bah_vol(declareRestartableData<std::vector<std::vector<Real>>>("bond_associated_horizon_vol")),
    _bah_vol_sum(declareRestartableData<std::vector<Real>>("bond_associated_horizon_vol_sum"))
{
  if (isParamValid("horizon_radius") && _has_horizon_number)
    mooseError(
        "You can specify only one option: horizon_radius or horizon_number in the mesh block!");

  if (!isParamValid("horizon_radius") && !_has_horizon_number)
    mooseError("You must specify one option: horizon_radius or horizon_number in the mesh block!");

  if (_has_cracks)
  {
    _cracks_start = getParam<std::vector<Point>>("cracks_start");
    _cracks_end = getParam<std::vector<Point>>("cracks_end");
  }
  else
  {
    _cracks_start.push_back(Point(0., 0., 0.));
    _cracks_end.push_back(Point(0., 0., 0.));
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

PeridynamicsMesh::~PeridynamicsMesh() { _node_coord.clear(); }

std::unique_ptr<MooseMesh>
PeridynamicsMesh::safeClone() const
{
  return libmesh_make_unique<PeridynamicsMesh>(*this);
}

void
PeridynamicsMesh::buildMesh()
{
  _mesh = _app.getMeshGeneratorMesh();
  _mesh->prepare_for_use(/*skip_renumber =*/true);
}

unsigned int
PeridynamicsMesh::dimension() const
{
  return _dim;
}

dof_id_type
PeridynamicsMesh::nPDNodes() const
{
  return _total_nodes;
}

dof_id_type
PeridynamicsMesh::nPDElems() const
{
  return _total_bonds;
}

void
PeridynamicsMesh::createExtraPeridynamicsMeshData(MeshBase & fe_mesh)
{
  // create unconventional mesh data for peridynamics mesh
  _dim = fe_mesh.mesh_dimension();
  _total_nodes = fe_mesh.n_elem();

  _node_coord.resize(_total_nodes);
  _node_mesh_spacing.resize(_total_nodes);
  _node_horizon.resize(_total_nodes);
  _node_vol.resize(_total_nodes);
  _node_horizon_vol.resize(_total_nodes);
  _node_blockID.resize(_total_nodes);
  _horizon_neighbors.resize(_total_nodes);
  _node_associated_bonds.resize(_total_nodes);
  _bah_dgneighbors.resize(_total_nodes);
  _bah_vol.resize(_total_nodes);
  _bah_vol_sum.resize(_total_nodes);

  Real dist = 0.0;
  // loop through all fe elements to generate PD nodes structure
  for (MeshBase::element_iterator it = fe_mesh.elements_begin(); it != fe_mesh.elements_end(); ++it)
  {
    Elem * fe_elem = *it;
    // calculate the mesh spacing as average distance between fe element with its neighbors
    unsigned int nneighbors = 0;
    Real dist_sum = 0.0;
    for (unsigned int i = 0; i < fe_elem->n_neighbors(); ++i)
      if (fe_elem->neighbor_ptr(i) != NULL)
      {
        dist = (fe_elem->centroid() - fe_elem->neighbor_ptr(i)->centroid()).norm();
        dist_sum += dist;
        nneighbors += 1;
      }
    _node_coord[fe_elem->id()] = fe_elem->centroid();
    _node_mesh_spacing[fe_elem->id()] = dist_sum / nneighbors;
    _node_horizon[fe_elem->id()] =
        (_has_horizon_number ? _horizon_number * dist_sum / nneighbors : _horizon_radius);
    _node_vol[fe_elem->id()] = fe_elem->volume();
    _node_horizon_vol[fe_elem->id()] = 0.0;
    _node_blockID[fe_elem->id()] = fe_elem->subdomain_id();
  }

  // search node neighbors and create other nodal data
  createNodeHorizonBasedData();

  // setup nodewise data for bond-associated deformation gradient
  createBondAssocHorizonBasedData();

  _total_bonds = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    _total_bonds += _horizon_neighbors[i].size();
  _total_bonds /= 2;

  unsigned int k = 0;
  for (unsigned int i = 0; i < _total_nodes; ++i)
    for (unsigned int j = 0; j < _horizon_neighbors[i].size(); ++j)
      if (_horizon_neighbors[i][j] > i)
      {
        // build the bond list for each node
        _node_associated_bonds[i].push_back(k);
        _node_associated_bonds[_horizon_neighbors[i][j]].push_back(k);
        ++k;
      }
}

void
PeridynamicsMesh::createNodeHorizonBasedData()
{
  // search neighbors
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    Real dis = 0.0;
    for (unsigned int j = 0; j < _total_nodes; ++j)
    {
      dis = (_node_coord[i] - _node_coord[j]).norm();
      if (_node_blockID[i] == _node_blockID[j] && dis <= 1.0001 * _node_horizon[i] && j != i)
      {
        // check whether pdnode i falls in the region whose bonds may need to be removed due to the
        // pre-existing cracks
        bool intersect = false;
        for (unsigned int k = 0; k < _cracks_start.size(); ++k)
        {
          if (checkPointInsideRectangle(_node_coord[i],
                                        _cracks_start[k],
                                        _cracks_end[k],
                                        _cracks_width[k] + 4.0 * _node_horizon[i],
                                        4.0 * _node_horizon[i]))
            intersect = intersect || checkCrackIntersectBond(_cracks_start[k],
                                                             _cracks_end[k],
                                                             _cracks_width[k],
                                                             _node_coord[i],
                                                             _node_coord[j]);
        }
        // remove bonds cross the crack to form crack surface
        if (!intersect)
        {
          // Use the addition balance scheme to remove unbalanced interactions
          // check whether j was already considered as a neighbor of i, if not, add j to i's
          // neighborlist
          if (std::find(_horizon_neighbors[i].begin(), _horizon_neighbors[i].end(), j) ==
              _horizon_neighbors[i].end())
          {
            _horizon_neighbors[i].push_back(j);
            _node_horizon_vol[i] += _node_vol[j];
          }
          // check whether i was also considered as a neighbor of j, if not, add i to j's
          // neighborlist
          if (std::find(_horizon_neighbors[j].begin(), _horizon_neighbors[j].end(), i) ==
              _horizon_neighbors[j].end())
          {
            _horizon_neighbors[j].push_back(i);
            _node_horizon_vol[j] += _node_vol[i];
          }
        }
      }
    }
  }
}

void
PeridynamicsMesh::createBondAssocHorizonBasedData()
{
  for (unsigned int i = 0; i < _total_nodes; ++i)
  {
    std::vector<dof_id_type> neighbors = _horizon_neighbors[i];
    _bah_dgneighbors[i].resize(neighbors.size());
    _bah_vol[i].resize(neighbors.size());
    _bah_vol_sum[i] = 0.0;
    for (unsigned int j = 0; j < neighbors.size(); ++j)
    {
      _bah_vol[i][j] = 0.0;
      for (unsigned int k = 0; k < neighbors.size(); ++k)
      {
        if ((_node_coord[neighbors[j]] - _node_coord[neighbors[k]]).norm() <=
            _horizons_ratio * _node_horizon[i])
        {
          // only save the corresponding index in neighbor list, rather than the actual node id
          _bah_dgneighbors[i][j].push_back(k);
          _bah_vol[i][j] += _node_vol[neighbors[k]];
          _bah_vol_sum[i] += _node_vol[neighbors[k]];
        }
      }
    }
  }
}

std::vector<dof_id_type>
PeridynamicsMesh::getNeighbors(dof_id_type node_id)
{
  return _horizon_neighbors[node_id];
}

unsigned int
PeridynamicsMesh::getNeighborID(dof_id_type node_i, dof_id_type node_j)
{
  std::vector<dof_id_type> neighbors = _horizon_neighbors[node_i];
  auto it = std::find(neighbors.begin(), neighbors.end(), node_j);
  if (it != neighbors.end())
    return it - neighbors.begin();
  else
    mooseError(
        "Material point ", node_j, " is not in the neighbor list of material point ", node_i);

  return -1;
}

std::vector<dof_id_type>
PeridynamicsMesh::getAssocBonds(dof_id_type node_id)
{
  return _node_associated_bonds[node_id];
}

std::vector<unsigned int>
PeridynamicsMesh::getBondAssocHorizonNeighbors(dof_id_type node_id, unsigned int neighbor_id)
{
  return _bah_dgneighbors[node_id][neighbor_id];
}

unsigned int
PeridynamicsMesh::getNodeBlockID(dof_id_type node_id)
{
  return _node_blockID[node_id];
}

Real
PeridynamicsMesh::getVolume(dof_id_type node_id)
{
  return _node_vol[node_id];
}

Real
PeridynamicsMesh::getHorizonVolume(dof_id_type node_id)
{
  return _node_horizon_vol[node_id];
}

Real
PeridynamicsMesh::getBondAssocHorizonVolume(dof_id_type node_id, unsigned int neighbor_id)
{
  return _bah_vol[node_id][neighbor_id];
}

Real
PeridynamicsMesh::getBondAssocHorizonVolumeSum(dof_id_type node_id)
{
  return _bah_vol_sum[node_id];
}

unsigned int
PeridynamicsMesh::getNNeighbors(dof_id_type node_id)
{
  return _horizon_neighbors[node_id].size();
}

Real
PeridynamicsMesh::getMeshSpacing(dof_id_type node_id)
{
  return _node_mesh_spacing[node_id];
}

Real
PeridynamicsMesh::getHorizon(dof_id_type node_id)
{
  return _node_horizon[node_id];
}

bool
PeridynamicsMesh::checkPointInsideRectangle(
    Point point, Point rec_p1, Point rec_p2, Real rec_height, Real tol)
{
  Real crack_length = (rec_p2 - rec_p1).norm();
  bool inside = crack_length;

  if (inside)
  {
    Real a = rec_p2(1) - rec_p1(1);
    Real b = rec_p1(0) - rec_p2(0);
    Real c = rec_p2(0) * rec_p1(1) - rec_p2(1) * rec_p1(0);
    inside *= std::abs(a * point(0) + b * point(1) + c) / crack_length < rec_height / 2.0;

    a = rec_p2(0) - rec_p1(0);
    b = rec_p2(1) - rec_p1(1);
    c = 0.5 * (rec_p1(1) * rec_p1(1) - rec_p2(1) * rec_p2(1) - rec_p2(0) * rec_p2(0) +
               rec_p1(0) * rec_p1(0));
    inside *= std::abs(a * point(0) + b * point(1) + c) / crack_length < (tol + crack_length) / 2.0;
  }

  return inside;
}

bool
PeridynamicsMesh::checkCrackIntersectBond(
    Point crack_p1, Point crack_p2, Real crack_width, Point bond_p1, Point bond_p2)
{
  bool intersect0 = false;
  bool intersect1 = false;
  bool intersect2 = false;
  if ((crack_p2 - crack_p1).norm())
  {
    intersect0 = checkSegmentIntersectSegment(crack_p1, crack_p2, bond_p1, bond_p2);
    if (crack_width != 0.)
    {
      Real crack_len = (crack_p1 - crack_p2).norm();
      Real cos_crack = (crack_p2(0) - crack_p1(0)) / crack_len;
      Real sin_crack = (crack_p2(1) - crack_p1(1)) / crack_len;
      Real new_crack_p1x = crack_p1(0) - crack_width / 2.0 * sin_crack;
      Real new_crack_p1y = crack_p1(1) + crack_width / 2.0 * cos_crack;
      Real new_crack_p2x = crack_p2(0) - crack_width / 2.0 * sin_crack;
      Real new_crack_p2y = crack_p2(1) + crack_width / 2.0 * cos_crack;
      Point new_crack_p1 = Point(new_crack_p1x, new_crack_p1y, 0.);
      Point new_crack_p2 = Point(new_crack_p2x, new_crack_p2y, 0.);
      intersect1 = checkSegmentIntersectSegment(new_crack_p1, new_crack_p2, bond_p1, bond_p2);
      new_crack_p1x = crack_p1(0) + crack_width / 2.0 * sin_crack;
      new_crack_p1y = crack_p1(1) - crack_width / 2.0 * cos_crack;
      new_crack_p2x = crack_p2(0) + crack_width / 2.0 * sin_crack;
      new_crack_p2y = crack_p2(1) - crack_width / 2.0 * cos_crack;
      new_crack_p1 = Point(new_crack_p1x, new_crack_p1y, 0.);
      new_crack_p2 = Point(new_crack_p2x, new_crack_p2y, 0.);
      intersect2 = checkSegmentIntersectSegment(new_crack_p1, new_crack_p2, bond_p1, bond_p2);
    }
  }

  return intersect0 || intersect1 || intersect2;
}

bool
PeridynamicsMesh::checkSegmentIntersectSegment(Point seg1_p1,
                                               Point seg1_p2,
                                               Point seg2_p1,
                                               Point seg2_p2)
{
  // Fail if the segments share an end-point
  if ((seg1_p1(0) == seg2_p1(0) && seg1_p1(1) == seg2_p1(1)) ||
      (seg1_p2(0) == seg2_p1(0) && seg1_p2(1) == seg2_p1(1)) ||
      (seg1_p1(0) == seg2_p2(0) && seg1_p1(1) == seg2_p2(1)) ||
      (seg1_p2(0) == seg2_p2(0) && seg1_p2(1) == seg2_p2(1)))
  {
    return false;
  }

  // Fail if the segments intersect at a given end-point but not normal to the crack
  if ((seg1_p1(1) - seg1_p2(1)) / (seg1_p1(0) - seg1_p2(0)) ==
          (seg1_p1(1) - seg2_p1(1)) / (seg1_p1(0) - seg2_p1(0)) ||
      (seg1_p1(1) - seg1_p2(1)) / (seg1_p1(0) - seg1_p2(0)) ==
          (seg1_p1(1) - seg2_p2(1)) / (seg1_p1(0) - seg2_p2(0)) ||
      (seg2_p1(1) - seg2_p2(1)) / (seg2_p1(0) - seg2_p2(0)) ==
          (seg2_p1(1) - seg1_p1(1)) / (seg2_p1(0) - seg1_p1(0)) ||
      (seg2_p1(1) - seg2_p2(1)) / (seg2_p1(0) - seg2_p2(0)) ==
          (seg2_p1(1) - seg1_p2(1)) / (seg2_p1(0) - seg1_p2(0)))
  {
    Real COSseg1_seg2 = (seg1_p2 - seg1_p1) * (seg2_p2 - seg2_p1) /
                        ((seg1_p2 - seg1_p1).norm() * (seg2_p2 - seg2_p1).norm());
    if (COSseg1_seg2 > -0.08715574 && COSseg1_seg2 < 0.08715574)
      return false;
  }

  // Translate the system so that point seg1_p1 is on the origin
  seg1_p2(0) -= seg1_p1(0);
  seg1_p2(1) -= seg1_p1(1);
  seg2_p1(0) -= seg1_p1(0);
  seg2_p1(1) -= seg1_p1(1);
  seg2_p2(0) -= seg1_p1(0);
  seg2_p2(1) -= seg1_p1(1);

  // Length of segment seg1_p1-seg1_p2
  Real seg1_len = seg1_p2.norm();

  // Rotate the system so that point seg1_p2 is on the positive X axis
  Real cos_seg1 = seg1_p2(0) / seg1_len;
  Real sin_seg1 = seg1_p2(1) / seg1_len;
  Real newX = seg2_p1(0) * cos_seg1 + seg2_p1(1) * sin_seg1;
  seg2_p1(1) = seg2_p1(1) * cos_seg1 - seg2_p1(0) * sin_seg1;
  seg2_p1(0) = newX;
  newX = seg2_p2(0) * cos_seg1 + seg2_p2(1) * sin_seg1;
  seg2_p2(1) = seg2_p2(1) * cos_seg1 - seg2_p2(0) * sin_seg1;
  seg2_p2(0) = newX;

  // Fail if segment seg2_p1-seg2_p2 doesn't cross segment seg1_p1-seg1_p2
  if ((seg2_p1(1) < 0. && seg2_p2(1) < 0.) || (seg2_p1(1) >= 0. && seg2_p2(1) >= 0.))
    return false;

  // Fail if segment seg2_p1-seg2_p2 crosses segment seg1_p1-seg1_p2 outside of segment
  // seg1_p1-seg1_p2
  Real seg1_pos = seg2_p2(0) + (seg2_p1(0) - seg2_p2(0)) * seg2_p2(1) / (seg2_p2(1) - seg2_p1(1));
  if (seg1_pos < 0. || seg1_pos > seg1_len)
    return false;

  return true;
}
