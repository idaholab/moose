//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AzimuthalBlockSplitGenerator.h"
#include "MooseMeshUtils.h"
#include "PolygonalMeshGenerationUtils.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("ReactorApp", AzimuthalBlockSplitGenerator);

InputParameters
AzimuthalBlockSplitGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "old_blocks", "The list of blocks in the input mesh that need to be modified.");
  params.addRequiredParam<std::vector<subdomain_id_type>>(
      "new_block_ids", "The block IDs to be used for the new selected azimuthal angle blocks.");
  params.addParam<std::vector<SubdomainName>>(
      "new_block_names",
      {},
      "The optional block names to be used for the new selected azimulathal angle blocks.");
  params.addParam<bool>("preserve_volumes",
                        true,
                        "Volume of concentric circles can be preserved using this function.");
  params.addRequiredRangeCheckedParam<Real>("start_angle",
                                            "start_angle>=0.0 & start_angle<360.0",
                                            "Starting azimuthal angle of the new block.");
  params.addRequiredRangeCheckedParam<Real>("angle_range",
                                            "angle_range>0.0 & angle_range<=180.0",
                                            "Azimuthal angle range of the new block.");
  params.addClassDescription(
      "This AzimuthalBlockSplitGenerator object takes in a polygon/hexagon concentric circle mesh "
      "and renames blocks on a user-defined azimuthal segment / wedge of the mesh.");

  return params;
}

AzimuthalBlockSplitGenerator::AzimuthalBlockSplitGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _new_block_ids(getParam<std::vector<subdomain_id_type>>("new_block_ids")),
    _new_block_names(getParam<std::vector<SubdomainName>>("new_block_names")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _start_angle(getParam<Real>("start_angle") + 90.0),
    _angle_range(getParam<Real>("angle_range")),
    _azimuthal_angle_meta(
        declareMeshProperty<std::vector<Real>>("azimuthal_angle_meta", std::vector<Real>())),
    _input(getMeshByName(_input_name))
{
  declareMeshProperty<Real>("pattern_pitch_meta", 0.0);
  declareMeshProperty<bool>("is_control_drum_meta", true);
  if (!_new_block_names.empty() && _new_block_names.size() != _new_block_ids.size())
    paramError("new_block_names",
               "This parameter, if provided, must have the same size as new_block_ids.");
  _num_sectors_per_side_meta =
      getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", _input_name);
  _start_angle = _start_angle >= 360.0 ? _start_angle - 360.0 : _start_angle;
  _end_angle = (_start_angle + _angle_range >= 360.0) ? (_start_angle + _angle_range - 360.0)
                                                      : (_start_angle + _angle_range);
}

std::unique_ptr<MeshBase>
AzimuthalBlockSplitGenerator::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;

  // Check the order of the input mesh's elements
  // Meanwhile, record the vertex average of each element for future comparison
  unsigned short order = 0;
  std::map<dof_id_type, Point> vertex_avgs;
  for (const auto & elem : mesh.element_ptr_range())
  {
    switch (elem->type())
    {
      case TRI3:
      case QUAD4:
        if (order == 2)
          paramError("input", "This mesh contains elements of different orders.");
        order = 1;
        break;
      case TRI6:
      case TRI7:
      case QUAD8:
      case QUAD9:
        if (order == 1)
          paramError("input", "This mesh contains elements of different orders.");
        order = 2;
        break;
      default:
        paramError("input", "This mesh contains elements of unsupported type.");
    }
    vertex_avgs[elem->id()] = elem->vertex_average();
  }

  _old_block_ids =
      MooseMeshUtils::getSubdomainIDs(mesh, getParam<std::vector<SubdomainName>>("old_blocks"));
  if (_old_block_ids.size() != _new_block_ids.size())
    paramError("new_block_ids", "This parameter must have the same size as old_blocks.");

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  mesh.subdomain_ids(mesh_blocks);

  for (unsigned int i = 0; i < _old_block_ids.size(); i++)
    if (_old_block_ids[i] == Moose::INVALID_BLOCK_ID || !mesh_blocks.count(_old_block_ids[i]))
      paramError("old_blocks",
                 "This parameter contains blocks that do not exist in the input mesh. Error "
                 "triggered by block: " +
                     getParam<std::vector<SubdomainName>>("old_blocks")[i]);

  if (std::find(_old_block_ids.begin(),
                _old_block_ids.end(),
                getMeshProperty<subdomain_id_type>("quad_center_block_id", _input_name)) !=
      _old_block_ids.end())
    paramError(
        "old_blocks",
        "This parameter contains a block that involves center quad elements, azimuthal splitting "
        "is currently not supported in this case.");

  MeshTools::Modification::rotate(mesh, 90.0, 0.0, 0.0);
  _azimuthal_angle_meta = azimuthalAnglesCollector(mesh, -180.0, 180.0, ANGLE_DEGREE);
  std::vector<Real> azimuthal_angles_vtx;
  const bool is_first_value_vertex =
      order == 1 ? true : MooseUtils::absoluteFuzzyEqual(_azimuthal_angle_meta[0], -180.0);
  if (order == 1)
    azimuthal_angles_vtx = _azimuthal_angle_meta;
  else
    for (const auto & i : make_range(_azimuthal_angle_meta.size()))
      if (i % 2 != is_first_value_vertex)
        azimuthal_angles_vtx.push_back(_azimuthal_angle_meta[i]);

  // So that this class works for both derived classes of PolygonMeshGeneratorBase
  auto pattern_pitch_meta = std::max(getMeshProperty<Real>("pitch_meta", _input_name),
                                     getMeshProperty<Real>("pattern_pitch_meta", _input_name));
  setMeshProperty("pattern_pitch_meta", pattern_pitch_meta);

  Real radiusCorrectionFactor_original =
      _preserve_volumes ? PolygonalMeshGenerationUtils::radiusCorrectionFactor(
                              _azimuthal_angle_meta, true, order, is_first_value_vertex)
                        : 1.0;

  const Real azi_tol = 1.0E-6;
  const Real rad_tol = 1.0e-6;
  const Real side_angular_range = 360.0 / (Real)_num_sectors_per_side_meta.size();
  const Real side_angular_shift =
      _num_sectors_per_side_meta.size() % 2 == 0 ? 0.0 : side_angular_range / 2.0;

  _start_angle = _start_angle > 180.0 ? _start_angle - 360.0 : _start_angle;
  _end_angle = _end_angle > 180.0 ? _end_angle - 360.0 : _end_angle;

  Real original_start_down;
  std::vector<Real>::iterator original_start_down_it;
  Real original_start_up;
  std::vector<Real>::iterator original_start_up_it;

  // Identify the mesh azimuthal angles of the elements that need to be modified for the starting
  // edge of the absorber region.
  angleIdentifier(_start_angle,
                  original_start_down,
                  original_start_down_it,
                  original_start_up,
                  original_start_up_it,
                  azimuthal_angles_vtx);

  Real original_end_down;
  std::vector<Real>::iterator original_end_down_it;
  Real original_end_up;
  std::vector<Real>::iterator original_end_up_it;

  // Identify the mesh azimuthal angles of the elements that need to be modified for the ending
  // edge of the absorber region.
  angleIdentifier(_end_angle,
                  original_end_down,
                  original_end_down_it,
                  original_end_up,
                  original_end_up_it,
                  azimuthal_angles_vtx);

  Real azi_to_mod_start;
  Real azi_to_keep_start;

  // For the two mesh azimuthal angles identified, determine which need to be moved to the starting
  // edge position.
  angleModifier(side_angular_shift,
                side_angular_range,
                azi_tol,
                _start_angle,
                original_start_down,
                original_start_down_it,
                original_start_up,
                original_start_up_it,
                azi_to_keep_start,
                azi_to_mod_start);

  Real azi_to_mod_end;
  Real azi_to_keep_end;

  // For the two mesh azimuthal angles identified, determine which need to be moved to the ending
  // edge position.
  angleModifier(side_angular_shift,
                side_angular_range,
                azi_tol,
                _end_angle,
                original_end_down,
                original_end_down_it,
                original_end_up,
                original_end_up_it,
                azi_to_keep_end,
                azi_to_mod_end);

  if (azi_to_mod_end == azi_to_mod_start)
    paramError("angle_range",
               "The azimuthal intervals of the input mesh are too coarse for this parameter.");

  const auto & side_list = mesh.get_boundary_info().build_side_list();

  // See if the block that contains the external boundary is modified or not
  bool external_block_change(false);
  for (unsigned int i = 0; i < side_list.size(); i++)
  {
    if (std::get<2>(side_list[i]) == OUTER_SIDESET_ID)
    {
      dof_id_type block_id_tmp = mesh.elem_ref(std::get<0>(side_list[i])).subdomain_id();
      if (std::find(_old_block_ids.begin(), _old_block_ids.end(), block_id_tmp) !=
          _old_block_ids.end())
        external_block_change = true;
    }
  }

  mesh.get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      mesh.get_boundary_info().build_node_list();

  std::vector<std::pair<Real, dof_id_type>> node_id_keep_start;
  std::vector<std::pair<Real, dof_id_type>> node_id_mod_start;
  std::vector<std::pair<Real, dof_id_type>> node_id_keep_end;
  std::vector<std::pair<Real, dof_id_type>> node_id_mod_end;

  // Determine which nodes are involved in the elements that are intercepted by the starting/ending
  // angles
  Real max_quad_elem_rad(0.0);
  if (mesh.elem_ref(0).n_vertices() == 4)
    for (dof_id_type i = 0;
         i < (_num_sectors_per_side_meta[0] / 2 + 1) * (_num_sectors_per_side_meta[0] / 2 + 1);
         i++)
      max_quad_elem_rad =
          max_quad_elem_rad < mesh.node_ref(i).norm() ? mesh.node_ref(i).norm() : max_quad_elem_rad;
  for (const auto & node_ptr : as_range(mesh.nodes_begin(), mesh.nodes_end()))
  {
    const Node & node = *node_ptr;
    Real node_azi = atan2(node(1), node(0)) * 180.0 / M_PI;
    Real node_rad = std::sqrt(node(0) * node(0) + node(1) * node(1));
    if (node_rad > max_quad_elem_rad + rad_tol &&
        (std::abs(node_azi - azi_to_mod_start) < azi_tol ||
         std::abs(std::abs(node_azi - azi_to_mod_start) - 360.0) < azi_tol))
      node_id_mod_start.push_back(std::make_pair(node_rad, node.id()));
    if (node_rad > max_quad_elem_rad + rad_tol &&
        (std::abs(node_azi - azi_to_keep_start) < azi_tol ||
         std::abs(std::abs(node_azi - azi_to_keep_start) - 360.0) < azi_tol))
      node_id_keep_start.push_back(std::make_pair(node_rad, node.id()));
    if (node_rad > max_quad_elem_rad + rad_tol &&
        (std::abs(node_azi - azi_to_mod_end) < azi_tol ||
         std::abs(std::abs(node_azi - azi_to_mod_end) - 360.0) < azi_tol))
      node_id_mod_end.push_back(std::make_pair(node_rad, node.id()));
    if (node_rad > max_quad_elem_rad + rad_tol &&
        (std::abs(node_azi - azi_to_keep_end) < azi_tol ||
         std::abs(std::abs(node_azi - azi_to_keep_end) - 360.0) < azi_tol))
      node_id_keep_end.push_back(std::make_pair(node_rad, node.id()));
  }
  // Sort the involved nodes using radius as the key; this facilitates the determination of circular
  // regions nodes
  std::sort(node_id_mod_start.begin(), node_id_mod_start.end());
  std::sort(node_id_keep_start.begin(), node_id_keep_start.end());
  std::sort(node_id_mod_end.begin(), node_id_mod_end.end());
  std::sort(node_id_keep_end.begin(), node_id_keep_end.end());
  std::vector<Real> circular_rad_list;
  std::vector<Real> non_circular_rad_list;

  // Modify the nodes with the azimuthal angles identified before.
  nodeModifier(mesh,
               node_id_mod_start,
               node_id_keep_start,
               circular_rad_list,
               non_circular_rad_list,
               node_list,
               _start_angle,
               external_block_change,
               rad_tol);
  nodeModifier(mesh,
               node_id_mod_end,
               node_id_keep_end,
               circular_rad_list,
               non_circular_rad_list,
               node_list,
               _end_angle,
               external_block_change,
               rad_tol);

  const Real max_circular_radius =
      *std::max_element(circular_rad_list.begin(), circular_rad_list.end());
  const Real min_non_circular_radius =
      *std::min_element(non_circular_rad_list.begin(), non_circular_rad_list.end());

  // Before re-correcting the radii, correct the mid-point of the elements that are altered
  if (order == 2)
    for (const auto & elem : mesh.element_ptr_range())
    {
      const Point & original_vertex_avg = vertex_avgs[elem->id()];
      const Point & new_vertex_avg = elem->vertex_average();
      if (MooseUtils::absoluteFuzzyGreaterThan((original_vertex_avg - new_vertex_avg).norm(), 0.0))
      {
        if (elem->type() == TRI6 || elem->type() == TRI7)
        {
          *elem->node_ptr(3) = midPointCorrector(
              *elem->node_ptr(0), *elem->node_ptr(1), max_circular_radius, rad_tol);
          *elem->node_ptr(4) = midPointCorrector(
              *elem->node_ptr(1), *elem->node_ptr(2), max_circular_radius, rad_tol);
          *elem->node_ptr(5) = midPointCorrector(
              *elem->node_ptr(2), *elem->node_ptr(0), max_circular_radius, rad_tol);
          if (elem->type() == TRI7)
            *elem->node_ptr(6) = (*elem->node_ptr(0) + *elem->node_ptr(1) + *elem->node_ptr(2) +
                                  *elem->node_ptr(3) + *elem->node_ptr(4) + *elem->node_ptr(5)) /
                                 6.0;
        }
        else if (elem->type() == QUAD8 || elem->type() == QUAD9)
        {
          *elem->node_ptr(4) = midPointCorrector(
              *elem->node_ptr(0), *elem->node_ptr(1), max_circular_radius, rad_tol);
          *elem->node_ptr(5) = midPointCorrector(
              *elem->node_ptr(1), *elem->node_ptr(2), max_circular_radius, rad_tol);
          *elem->node_ptr(6) = midPointCorrector(
              *elem->node_ptr(2), *elem->node_ptr(3), max_circular_radius, rad_tol);
          *elem->node_ptr(7) = midPointCorrector(
              *elem->node_ptr(3), *elem->node_ptr(0), max_circular_radius, rad_tol);
          if (elem->type() == QUAD9)
            *elem->node_ptr(8) = (*elem->node_ptr(0) + *elem->node_ptr(1) + *elem->node_ptr(2) +
                                  *elem->node_ptr(3) + *elem->node_ptr(4) + *elem->node_ptr(5) +
                                  *elem->node_ptr(6) + *elem->node_ptr(7)) /
                                 8.0;
        }
      }
    }

  std::vector<Real> new_azimuthal_angle;
  for (const auto & node_ptr : as_range(mesh.nodes_begin(), mesh.nodes_end()))
  {
    const Node & node = *node_ptr;
    if (MooseUtils::absoluteFuzzyEqual(
            std::sqrt(node(0) * node(0) + node(1) * node(1)), max_circular_radius, rad_tol))
    {
      Real node_azi = atan2(node(1), node(0)) * 180.0 / M_PI;
      new_azimuthal_angle.push_back(node_azi);
    }
  }
  std::sort(new_azimuthal_angle.begin(), new_azimuthal_angle.end());
  _azimuthal_angle_meta = new_azimuthal_angle;
  const bool is_first_value_vertex_mod =
      order == 1 ? true : MooseUtils::absoluteFuzzyEqual(_azimuthal_angle_meta[0], -180.0);

  Real radiusCorrectionFactor_mod =
      _preserve_volumes ? PolygonalMeshGenerationUtils::radiusCorrectionFactor(
                              _azimuthal_angle_meta, true, order, is_first_value_vertex_mod)
                        : 1.0;

  // Re-correct Radii
  if (_preserve_volumes)
  {
    if (min_non_circular_radius <
        max_circular_radius / radiusCorrectionFactor_original * radiusCorrectionFactor_mod)
      mooseError("In AzimuthalBlockSplitGenerator ",
                 _name,
                 ": the circular region is overlapped with background region after correction.");
    for (Node * node_ptr : as_range(mesh.nodes_begin(), mesh.nodes_end()))
    {
      Node & node = *node_ptr;
      Real node_rad = std::sqrt(node(0) * node(0) + node(1) * node(1));
      // Any nodes with radii smaller than the threshold belong to circular regions
      if (node_rad > rad_tol && node_rad <= max_circular_radius + rad_tol)
      {
        const Real node_azi = atan2(node(1), node(0));
        const Real node_rad_corr =
            node_rad / radiusCorrectionFactor_original * radiusCorrectionFactor_mod;
        node(0) = node_rad_corr * std::cos(node_azi);
        node(1) = node_rad_corr * std::sin(node_azi);
      }
    }
  }
  // Assign New Block IDs
  for (unsigned int block_id_index = 0; block_id_index < _old_block_ids.size(); block_id_index++)
    for (auto & elem :
         as_range(mesh.active_subdomain_elements_begin(_old_block_ids[block_id_index]),
                  mesh.active_subdomain_elements_end(_old_block_ids[block_id_index])))
    {
      auto p_cent = elem->true_centroid();
      auto p_cent_azi = atan2(p_cent(1), p_cent(0)) * 180.0 / M_PI;
      if (_start_angle < _end_angle && p_cent_azi >= _start_angle && p_cent_azi <= _end_angle)
        elem->subdomain_id() = _new_block_ids[block_id_index];
      else if (_start_angle > _end_angle &&
               (p_cent_azi >= _start_angle || p_cent_azi <= _end_angle))
        elem->subdomain_id() = _new_block_ids[block_id_index];
    }
  // Assign new Block Names if applicable
  for (unsigned int i = 0; i < _new_block_names.size(); i++)
    mesh.subdomain_name(_new_block_ids[i]) = _new_block_names[i];
  MeshTools::Modification::rotate(mesh, -90.0, 0.0, 0.0);
  for (unsigned int i = 0; i < _azimuthal_angle_meta.size(); i++)
    _azimuthal_angle_meta[i] = (_azimuthal_angle_meta[i] - 90.0 <= -180.0)
                                   ? (_azimuthal_angle_meta[i] + 270.0)
                                   : _azimuthal_angle_meta[i] - 90.0;
  std::sort(_azimuthal_angle_meta.begin(), _azimuthal_angle_meta.end());

  return std::move(_input);
}

void
AzimuthalBlockSplitGenerator::angleIdentifier(const Real & terminal_angle,
                                              Real & original_down,
                                              std::vector<Real>::iterator & original_down_it,
                                              Real & original_up,
                                              std::vector<Real>::iterator & original_up_it,
                                              std::vector<Real> & azimuthal_angles_vtx)
{

  auto term_up =
      std::lower_bound(azimuthal_angles_vtx.begin(), azimuthal_angles_vtx.end(), terminal_angle);
  if (term_up == azimuthal_angles_vtx.begin())
  {
    original_up = *term_up;
    original_up_it = term_up;
    original_down = -180.0;
    original_down_it = azimuthal_angles_vtx.end() - 1;
  }
  else if (term_up == azimuthal_angles_vtx.end())
  {
    original_down = azimuthal_angles_vtx.back();
    original_down_it = azimuthal_angles_vtx.end() - 1;
    original_up = 180.0;
    original_up_it = azimuthal_angles_vtx.begin();
  }
  else
  {
    original_down = *(term_up - 1);
    original_down_it = term_up - 1;
    original_up = *term_up;
    original_up_it = term_up;
  }
}

void
AzimuthalBlockSplitGenerator::angleModifier(const Real & side_angular_shift,
                                            const Real & side_angular_range,
                                            const Real & azi_tol,
                                            const Real & terminal_angle,
                                            const Real & original_down,
                                            std::vector<Real>::iterator & original_down_it,
                                            const Real & original_up,
                                            std::vector<Real>::iterator & original_up_it,
                                            Real & azi_to_keep,
                                            Real & azi_to_mod)
{
  // Half interval is used to help determine which of the two identified azimuthal angles needs to
  // be moved.
  const Real half_interval = (original_up - original_down) / 2.0;
  // In this case, the lower azimuthal angle matches a vertex position, while the target angle is
  // not overlapped with the same vertex position. Thus the lower azimuthal angle is not moved
  // anyway.
  if (std::abs((original_down + side_angular_shift) / side_angular_range -
               std::round((original_down + side_angular_shift) / side_angular_range)) < azi_tol &&
      std::abs(original_down - terminal_angle) > azi_tol)
  {
    azi_to_keep = original_down;
    azi_to_mod = original_up;
    *original_up_it = terminal_angle;
  }
  // In this case, the upper azimuthal angle matches a vertex position, while the target angle is
  // not overlapped with the same vertex position. Thus the upper azimuthal angle is not moved
  // anyway.
  else if (std::abs((original_up + side_angular_shift) / side_angular_range -
                    std::round((original_up + side_angular_shift) / side_angular_range)) <
               azi_tol &&
           std::abs(original_up - terminal_angle) > azi_tol)
  {
    azi_to_keep = original_up;
    azi_to_mod = original_down;
    *original_down_it = terminal_angle;
  }
  // Move upper azimuthal angle as it is closer to the target angle.
  else if (terminal_angle - original_down > half_interval)
  {
    azi_to_keep = original_down;
    azi_to_mod = original_up;
    *original_up_it = terminal_angle;
  }
  // Move lower azimuthal angle as it is closer to the target angle.
  else
  {
    azi_to_keep = original_up;
    azi_to_mod = original_down;
    *original_down_it = terminal_angle;
  }
}

void
AzimuthalBlockSplitGenerator::nodeModifier(
    ReplicatedMesh & mesh,
    const std::vector<std::pair<Real, dof_id_type>> & node_id_mod,
    const std::vector<std::pair<Real, dof_id_type>> & node_id_keep,
    std::vector<Real> & circular_rad_list,
    std::vector<Real> & non_circular_rad_list,
    const std::vector<std::tuple<dof_id_type, boundary_id_type>> & node_list,
    const Real & term_angle,
    const bool & external_block_change,
    const Real & rad_tol)
{
  for (unsigned int i = 0; i < node_id_mod.size(); i++)
  {
    // Circular regions, radius is not altered
    if (std::abs(node_id_mod[i].first - node_id_keep[i].first) < rad_tol)
    {
      Node & node_mod = mesh.node_ref(node_id_mod[i].second);
      circular_rad_list.push_back(node_id_mod[i].first);
      node_mod(0) = node_id_mod[i].first * std::cos(term_angle * M_PI / 180.0);
      node_mod(1) = node_id_mod[i].first * std::sin(term_angle * M_PI / 180.0);
    }
    else
    {
      non_circular_rad_list.push_back(node_id_mod[i].first);
      // For external boundary nodes, if the external block is not modified, the boundary nodes are
      // not moved.
      // Non-circular range, use intercept instead
      if (std::find(node_list.begin(),
                    node_list.end(),
                    std::make_tuple(node_id_mod[i].second, OUTER_SIDESET_ID)) == node_list.end() ||
          external_block_change)
      {
        Node & node_mod = mesh.node_ref(node_id_mod[i].second);
        const Node & node_keep = mesh.node_ref(node_id_keep[i].second);
        std::pair<Real, Real> pair_tmp = fourPointIntercept(
            std::make_pair(node_mod(0), node_mod(1)),
            std::make_pair(node_keep(0), node_keep(1)),
            std::make_pair(0.0, 0.0),
            std::make_pair(2.0 * node_id_mod[i].first * std::cos(term_angle * M_PI / 180.0),
                           2.0 * node_id_mod[i].first * std::sin(term_angle * M_PI / 180.0)));
        node_mod(0) = pair_tmp.first;
        node_mod(1) = pair_tmp.second;
      }
    }
  }
}

Point
AzimuthalBlockSplitGenerator::midPointCorrector(const Point vertex_0,
                                                const Point vertex_1,
                                                const Real max_radius,
                                                const Real rad_tol)
{
  // Check if two vertices have the same radius
  const Real r_vertex_0 = std::sqrt(vertex_0(0) * vertex_0(0) + vertex_0(1) * vertex_0(1));
  const Real r_vertex_1 = std::sqrt(vertex_1(0) * vertex_1(0) + vertex_1(1) * vertex_1(1));
  // If both vertices have the same radius and they are located in the circular region,
  // their midpoint should have the same radius and has the average azimuthal angle of the two
  // vertices.
  if (std::abs(r_vertex_0 - r_vertex_1) < rad_tol && r_vertex_0 < max_radius + rad_tol)
    return (vertex_0 + vertex_1).unit() * r_vertex_0;
  else
    return (vertex_0 + vertex_1) / 2.0;
}
