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

InputParameters
PeridynamicsMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addClassDescription("Mesh class to store and return peridynamics specific mesh data");

  params.addParam<Real>("horizon_radius", "Value of horizon size in terms of radius");
  params.addParam<Real>("horizon_number",
                        "The material points spacing number, i.e. ratio of horizon radius to the "
                        "effective average spacing");
  params.addParam<Real>("bond_associated_horizon_ratio",
                        1.5,
                        "Ratio of bond-associated horizon to nodal horizon. This is the only "
                        "parameters to control the size of bond-associated horizon");
  params.addParam<std::vector<Point>>("cracks_start",
                                      "Cartesian coordinates where predefined line cracks start");
  params.addParam<std::vector<Point>>("cracks_end",
                                      "Cartesian coordinates where predefined line cracks end");
  params.addParam<std::vector<Real>>("cracks_width", "Widths of predefined line cracks");

  params.set<bool>("_mesh_generator_mesh") = true;

  return params;
}

PeridynamicsMesh::PeridynamicsMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _horizon_radius(isParamValid("horizon_radius") ? getParam<Real>("horizon_radius") : 0),
    _has_horizon_number(isParamValid("horizon_number")),
    _horizon_number(_has_horizon_number ? getParam<Real>("horizon_number") : 0),
    _bah_ratio(getParam<Real>("bond_associated_horizon_ratio")),
    _has_cracks(isParamValid("cracks_start") || isParamValid("cracks_end")),
    _dim(declareRestartableData<unsigned int>("dim")),
    _n_pdnodes(declareRestartableData<unsigned int>("n_pdnodes")),
    _n_pdbonds(declareRestartableData<unsigned int>("n_pdbonds")),
    _pdnode_average_spacing(declareRestartableData<std::vector<Real>>("pdnode_average_spacing")),
    _pdnode_horizon_radius(declareRestartableData<std::vector<Real>>("pdnode_horizon_radius")),
    _pdnode_vol(declareRestartableData<std::vector<Real>>("pdnode_vol")),
    _pdnode_horizon_vol(declareRestartableData<std::vector<Real>>("pdnode_horizon_vol")),
    _pdnode_blockID(declareRestartableData<std::vector<SubdomainID>>("pdnode_blockID")),
    _pdnode_elemID(declareRestartableData<std::vector<dof_id_type>>("pdnode_elemID")),
    _pdnode_neighbors(
        declareRestartableData<std::vector<std::vector<dof_id_type>>>("pdnode_neighbors")),
    _pdnode_bonds(declareRestartableData<std::vector<std::vector<dof_id_type>>>("pdnode_bonds")),
    _dg_neighbors(
        declareRestartableData<std::vector<std::vector<std::vector<dof_id_type>>>>("dg_neighbors")),
    _pdnode_sub_vol(declareRestartableData<std::vector<std::vector<Real>>>("pdnode_sub_vol")),
    _pdnode_sub_vol_sum(declareRestartableData<std::vector<Real>>("pdnode_sub_vol_sum")),
    _boundary_node_offset(
        declareRestartableData<std::map<dof_id_type, Real>>("boundary_node_offset"))
{
  if (!(isParamValid("horizon_radius") || _has_horizon_number))
    mooseError("Must specify either horizon_radius or horizon_number to determine horizon size in "
               "the mesh block!");

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
    mooseError(
        "Number of cracks starting points is NOT the same as number of cracks ending points!");

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

std::unique_ptr<MooseMesh>
PeridynamicsMesh::safeClone() const
{
  return std::make_unique<PeridynamicsMesh>(*this);
}

void
PeridynamicsMesh::buildMesh()
{
  if (!hasMeshBase())
  {
    auto & entry = _app.getMeshGeneratorSystem();
    _mesh = entry.getSavedMesh(entry.mainMeshGeneratorName());
  }
  _mesh->allow_renumbering(false);
  _mesh->prepare_for_use();
  _mesh->allow_renumbering(true);
}

unsigned int
PeridynamicsMesh::dimension() const
{
  return _dim;
}

dof_id_type
PeridynamicsMesh::nPDNodes() const
{
  return _n_pdnodes;
}

dof_id_type
PeridynamicsMesh::nPDBonds() const
{
  return _n_pdbonds;
}

void
PeridynamicsMesh::createPeridynamicsMeshData(
    MeshBase & fe_mesh,
    std::set<dof_id_type> converted_elem_id,
    std::multimap<SubdomainID, SubdomainID> bonding_block_pairs,
    std::multimap<SubdomainID, SubdomainID> non_bonding_block_pairs)
{
  _dim = fe_mesh.mesh_dimension();
  _n_pdnodes = converted_elem_id.size();

  // initialize data size
  _pdnode_coord.resize(_n_pdnodes);
  _pdnode_average_spacing.resize(_n_pdnodes);
  _pdnode_horizon_radius.resize(_n_pdnodes);
  _pdnode_vol.resize(_n_pdnodes);
  _pdnode_horizon_vol.resize(_n_pdnodes);
  _pdnode_blockID.resize(_n_pdnodes);
  _pdnode_elemID.resize(_n_pdnodes);
  _pdnode_neighbors.resize(_n_pdnodes);
  _pdnode_bonds.resize(_n_pdnodes);
  _dg_neighbors.resize(_n_pdnodes);
  _pdnode_sub_vol.resize(_n_pdnodes);
  _pdnode_sub_vol_sum.resize(_n_pdnodes, 0.0);

  // loop through converted fe elements to generate PD nodes structure
  unsigned int id = 0; // make pd nodes start at 0 in the new mesh
  for (const auto & eid : converted_elem_id)
  {
    Elem * fe_elem = fe_mesh.elem_ptr(eid);
    // calculate the nodes spacing as average distance between fe element with its neighbors
    unsigned int n_fe_neighbors = 0;
    Real dist_sum = 0.0;
    for (unsigned int j = 0; j < fe_elem->n_neighbors(); ++j)
      if (fe_elem->neighbor_ptr(j) != nullptr)
      {
        dist_sum += (fe_elem->vertex_average() - fe_elem->neighbor_ptr(j)->vertex_average()).norm();
        n_fe_neighbors++;
      }
      else // this side is on boundary and calculate the distance to the centroid
      {
        Real dist = 0.0;
        std::vector<unsigned int> nid = fe_elem->nodes_on_side(j);
        Point p0 = fe_elem->vertex_average();
        Point p1 = fe_elem->point(nid[0]);
        if (fe_elem->dim() == 2) // 2D elems
        {
          Point p2 = fe_elem->point(nid.back());
          Real area = 0.5 * std::abs(p0(0) * (p1(1) - p2(1)) + p1(0) * (p2(1) - p0(1)) +
                                     p2(0) * (p0(1) - p1(1)));
          dist = 2.0 * area / fe_elem->length(nid[0], nid[1]);
        }
        else // 3D elems
        {
          Point p2 = fe_elem->point(nid[1]);
          Point p3 = fe_elem->point(nid.back());
          Point vec0 = p1 - p2;
          Point vec1 = p1 - p3;
          Point normal = vec0.cross(vec1);
          normal /= normal.norm();
          dist = std::abs(normal(0) * (p0(0) - p1(0)) + normal(1) * (p0(1) - p1(1)) +
                          normal(2) * (p0(2) - p1(2)));
        }
        _boundary_node_offset.insert(std::make_pair(id, -dist));
      }

    _pdnode_coord[id] = fe_elem->vertex_average();
    _pdnode_average_spacing[id] = dist_sum / n_fe_neighbors;
    _pdnode_horizon_radius[id] =
        (_has_horizon_number ? _horizon_number * dist_sum / n_fe_neighbors : _horizon_radius);
    // NOTE: PeridynamicsMesh does not support RZ/RSpherical so using volume from libmesh elem is
    // fine
    _pdnode_vol[id] = fe_elem->volume();
    _pdnode_horizon_vol[id] = 0.0;
    _pdnode_blockID[id] = fe_elem->subdomain_id() + 1000; // set new subdomain id for PD mesh in
                                                          //  case FE mesh is retained
    _pdnode_elemID[id] = fe_elem->id();

    ++id;
  }

  // search node neighbors and create other nodal data
  createNodeHorizBasedData(bonding_block_pairs, non_bonding_block_pairs);

  createNeighborHorizonBasedData(); // applies to non-ordinary state-based model only.

  // total number of peridynamic bonds
  _n_pdbonds = 0;
  for (unsigned int i = 0; i < _n_pdnodes; ++i)
    _n_pdbonds += _pdnode_neighbors[i].size();
  _n_pdbonds /= 2;

  unsigned int k = 0;
  for (unsigned int i = 0; i < _n_pdnodes; ++i)
    for (unsigned int j = 0; j < _pdnode_neighbors[i].size(); ++j)
      if (_pdnode_neighbors[i][j] > i)
      {
        // build the bond list for each node
        _pdnode_bonds[i].push_back(k);
        _pdnode_bonds[_pdnode_neighbors[i][j]].push_back(k);
        ++k;
      }
}

void
PeridynamicsMesh::createNodeHorizBasedData(
    std::multimap<SubdomainID, SubdomainID> bonding_block_pairs,
    std::multimap<SubdomainID, SubdomainID> non_bonding_block_pairs)
{
  // search neighbors
  for (unsigned int i = 0; i < _n_pdnodes; ++i)
  {
    Real dis = 0.0;
    for (unsigned int j = 0; j < _n_pdnodes; ++j)
    {
      dis = (_pdnode_coord[i] - _pdnode_coord[j]).norm();
      if (dis <= 1.0001 * _pdnode_horizon_radius[i] && j != i)
      {
        bool is_interface = false;
        if (!bonding_block_pairs.empty())
          is_interface =
              checkInterface(_pdnode_blockID[i], _pdnode_blockID[j], bonding_block_pairs);

        if (!non_bonding_block_pairs.empty())
          is_interface =
              !checkInterface(_pdnode_blockID[i], _pdnode_blockID[j], non_bonding_block_pairs);

        if (_pdnode_blockID[i] == _pdnode_blockID[j] || is_interface)
        {
          // check whether pdnode i falls in the region whose bonds may need to be removed due to
          // the pre-existing cracks
          bool intersect = false;
          for (unsigned int k = 0; k < _cracks_start.size(); ++k)
          {
            if (checkPointInsideRectangle(_pdnode_coord[i],
                                          _cracks_start[k],
                                          _cracks_end[k],
                                          _cracks_width[k] + 4.0 * _pdnode_horizon_radius[i],
                                          4.0 * _pdnode_horizon_radius[i]))
              intersect = intersect || checkCrackIntersectBond(_cracks_start[k],
                                                               _cracks_end[k],
                                                               _cracks_width[k],
                                                               _pdnode_coord[i],
                                                               _pdnode_coord[j]);
          }
          // remove bonds cross the crack to form crack surface
          if (!intersect)
          {
            // Use the addition balance scheme to remove unbalanced interactions
            // check whether j was already considered as a neighbor of i, if not, add j to i's
            // neighborlist
            if (std::find(_pdnode_neighbors[i].begin(), _pdnode_neighbors[i].end(), j) ==
                _pdnode_neighbors[i].end())
            {
              _pdnode_neighbors[i].push_back(j);
              _pdnode_horizon_vol[i] += _pdnode_vol[j];
            }
            // check whether i was also considered as a neighbor of j, if not, add i to j's
            // neighborlist
            if (std::find(_pdnode_neighbors[j].begin(), _pdnode_neighbors[j].end(), i) ==
                _pdnode_neighbors[j].end())
            {
              _pdnode_neighbors[j].push_back(i);
              _pdnode_horizon_vol[j] += _pdnode_vol[i];
            }
          }
        }
      }
    }
  }
}

bool
PeridynamicsMesh::checkInterface(SubdomainID pdnode_blockID_i,
                                 SubdomainID pdnode_blockID_j,
                                 std::multimap<SubdomainID, SubdomainID> blockID_pairs)
{
  bool is_interface = false;
  std::pair<std::multimap<SubdomainID, SubdomainID>::iterator,
            std::multimap<SubdomainID, SubdomainID>::iterator>
      ret;
  // check existence of the case when i is the key and j is the value
  ret = blockID_pairs.equal_range(pdnode_blockID_i);
  for (std::multimap<SubdomainID, SubdomainID>::iterator it = ret.first; it != ret.second; ++it)
    if (pdnode_blockID_j == it->second)
      is_interface = true;

  // check existence of the case when j is the key and i is the value
  ret = blockID_pairs.equal_range(pdnode_blockID_j);
  for (std::multimap<SubdomainID, SubdomainID>::iterator it = ret.first; it != ret.second; ++it)
    if (pdnode_blockID_i == it->second)
      is_interface = true;

  return is_interface;
}

void
PeridynamicsMesh::createNeighborHorizonBasedData()
{
  for (unsigned int i = 0; i < _n_pdnodes; ++i)
  {
    std::vector<dof_id_type> n_pd_neighbors = _pdnode_neighbors[i];
    _dg_neighbors[i].resize(n_pd_neighbors.size());
    _pdnode_sub_vol[i].resize(n_pd_neighbors.size(), 0.0);

    for (unsigned int j = 0; j < n_pd_neighbors.size(); ++j)
      for (unsigned int k = j; k < n_pd_neighbors.size(); ++k) // only search greater number index
        if ((_pdnode_coord[n_pd_neighbors[j]] - _pdnode_coord[n_pd_neighbors[k]]).norm() <=
            _bah_ratio * _pdnode_horizon_radius[i])
        {
          // only save the corresponding index in neighbor list, rather than the actual node id
          // for neighbor j
          _dg_neighbors[i][j].push_back(k);
          _pdnode_sub_vol[i][j] += _pdnode_vol[n_pd_neighbors[k]];
          _pdnode_sub_vol_sum[i] += _pdnode_vol[n_pd_neighbors[k]];
          // for neighbor k
          if (k > j)
          {
            _dg_neighbors[i][k].push_back(j);
            _pdnode_sub_vol[i][k] += _pdnode_vol[n_pd_neighbors[j]];
            _pdnode_sub_vol_sum[i] += _pdnode_vol[n_pd_neighbors[j]];
          }
        }
  }
}

std::vector<dof_id_type>
PeridynamicsMesh::getNeighbors(dof_id_type node_id)
{
  return _pdnode_neighbors[node_id];
}

dof_id_type
PeridynamicsMesh::getNeighborIndex(dof_id_type node_i, dof_id_type node_j)
{
  std::vector<dof_id_type> n_pd_neighbors = _pdnode_neighbors[node_i];
  auto it = std::find(n_pd_neighbors.begin(), n_pd_neighbors.end(), node_j);
  if (it != n_pd_neighbors.end())
    return it - n_pd_neighbors.begin();
  else
    mooseError(
        "Material point ", node_j, " is not in the neighbor list of material point ", node_i);

  return -1;
}

std::vector<dof_id_type>
PeridynamicsMesh::getBonds(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_bonds[node_id];
}

std::vector<dof_id_type>
PeridynamicsMesh::getBondDeformationGradientNeighbors(dof_id_type node_id, dof_id_type neighbor_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  if (neighbor_id > _pdnode_neighbors[node_id].size() - 1)
    mooseError("Querying neighbor index exceeds the available neighbors!");

  std::vector<dof_id_type> dg_neighbors = _dg_neighbors[node_id][neighbor_id];
  if (dg_neighbors.size() < _dim)
    mooseError("Not enough number of neighbors to calculate deformation gradient at PD node: ",
               node_id);

  return dg_neighbors;
}

SubdomainID
PeridynamicsMesh::getNodeBlockID(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_blockID[node_id];
}

void
PeridynamicsMesh::setNodeBlockID(SubdomainID id)
{
  _pdnode_blockID.assign(_n_pdnodes, id);
}

Point
PeridynamicsMesh::getNodeCoord(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_coord[node_id];
}

std::vector<dof_id_type>
PeridynamicsMesh::getPDNodeIDToFEElemIDMap()
{
  return _pdnode_elemID;
}

Real
PeridynamicsMesh::getNodeVolume(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_vol[node_id];
}

Real
PeridynamicsMesh::getHorizonVolume(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_horizon_vol[node_id];
}

Real
PeridynamicsMesh::getHorizonSubsetVolume(dof_id_type node_id, dof_id_type neighbor_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  if (neighbor_id > _pdnode_neighbors[node_id].size() - 1)
    mooseError("Querying neighbor index exceeds the available neighbors!");

  return _pdnode_sub_vol[node_id][neighbor_id];
}

Real
PeridynamicsMesh::getHorizonSubsetVolumeSum(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_sub_vol_sum[node_id];
}

Real
PeridynamicsMesh::getHorizonSubsetVolumeFraction(dof_id_type node_id, dof_id_type neighbor_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  if (neighbor_id > _pdnode_neighbors[node_id].size() - 1)
    mooseError("Querying neighbor index exceeds the available neighbors!");

  return _pdnode_sub_vol[node_id][neighbor_id] / _pdnode_sub_vol_sum[node_id];
}

Real
PeridynamicsMesh::getNodeAverageSpacing(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_average_spacing[node_id];
}

Real
PeridynamicsMesh::getHorizon(dof_id_type node_id)
{
  if (node_id > _n_pdnodes)
    mooseError("Querying node ID exceeds the available PD node IDs!");

  return _pdnode_horizon_radius[node_id];
}

Real
PeridynamicsMesh::getBoundaryOffset(dof_id_type node_id)
{
  if (_boundary_node_offset.count(node_id))
    return _boundary_node_offset[node_id];
  else
    return 0.0;
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
    inside =
        inside && (std::abs(a * point(0) + b * point(1) + c) / crack_length < rec_height / 2.0);

    a = rec_p2(0) - rec_p1(0);
    b = rec_p2(1) - rec_p1(1);
    c = 0.5 * (rec_p1(1) * rec_p1(1) - rec_p2(1) * rec_p2(1) - rec_p2(0) * rec_p2(0) +
               rec_p1(0) * rec_p1(0));
    inside = inside && (std::abs(a * point(0) + b * point(1) + c) / crack_length <
                        (tol + crack_length) / 2.0);
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
