//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AzimuthalBlockIDMeshGenerator.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("ReactorApp", AzimuthalBlockIDMeshGenerator);

InputParameters
AzimuthalBlockIDMeshGenerator::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");
  params.addParam<std::vector<subdomain_id_type>>(
      "old_block_ids", "The block IDs in the input mesh that need to be modified.");
  params.addParam<std::vector<SubdomainName>>(
      "old_block_names", "The block names in the input mesh that need to be modified.");
  params.addRequiredParam<std::vector<subdomain_id_type>>(
      "new_block_ids", "The block IDs to be used for the new selected azimulathal angle blocks.");
  params.addParam<std::vector<SubdomainName>>(
      "new_block_names",
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
      "This AzimuthalBlockIDMeshGenerator object takes in a polygon/hexagon concentric circle mesh "
      "and renames blocks on a user-defined azimuthal segment / wedge of the mesh.");

  return params;
}

AzimuthalBlockIDMeshGenerator::AzimuthalBlockIDMeshGenerator(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _old_block_ids(getParam<std::vector<subdomain_id_type>>("old_block_ids")),
    _old_block_names(getParam<std::vector<SubdomainName>>("old_block_names")),
    _new_block_ids(getParam<std::vector<subdomain_id_type>>("new_block_ids")),
    _new_block_names(getParam<std::vector<SubdomainName>>("new_block_names")),
    _preserve_volumes(getParam<bool>("preserve_volumes")),
    _start_angle(getParam<Real>("start_angle") + 90.0),
    _angle_range(getParam<Real>("angle_range")),
    _pattern_pitch_meta(declareMeshProperty<Real>("pattern_pitch_meta", 0.0)),
    _is_control_drum_meta(declareMeshProperty<bool>("is_control_drum_meta", true)),
    _azimuthal_angle_meta(
        declareMeshProperty<std::vector<Real>>("azimuthal_angle_meta", std::vector<Real>()))
{
  if ((unsigned int)_old_block_ids.empty() + (unsigned int)_old_block_names.empty() != 1)
    paramError("old_block_ids",
               "One and only one of old_block_ids and old_block_names must be given.");
  if (_old_block_ids.size() != _new_block_ids.size() && !_old_block_ids.empty())
    paramError("new_block_ids",
               "This parameter must have the same size as old_block_ids if it is given.");
  if (_old_block_names.size() != _new_block_ids.size() && !_old_block_names.empty())
    paramError("new_block_names",
               "This parameter must have the same size as old_block_names if it is given.");
  if (!_new_block_names.empty() && _new_block_names.size() != _new_block_ids.size())
    paramError("new_block_names",
               "This parameter, if provided, must have the same size as new_block_ids.");
  _mesh_ptrs = &getMeshByName(_input_name);
  _num_sectors_per_side_meta =
      getMeshProperty<std::vector<unsigned int>>("num_sectors_per_side_meta", _input_name);
  _start_angle = _start_angle >= 360.0 ? _start_angle - 360.0 : _start_angle;
  _end_angle = (_start_angle + _angle_range >= 360.0) ? (_start_angle + _angle_range - 360.0)
                                                      : (_start_angle + _angle_range);
}

std::unique_ptr<MeshBase>
AzimuthalBlockIDMeshGenerator::generate()
{
  _meshes = dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs);

  if (!_old_block_names.empty())
  {
    std::map<subdomain_id_type, std::string> original_subdomain_map =
        _meshes->get_subdomain_name_map();
    for (unsigned int i = 0; i < _old_block_names.size(); i++)
    {
      const std::string findVal = _old_block_names[i];
      auto it = std::find_if(original_subdomain_map.begin(),
                             original_subdomain_map.end(),
                             [findVal](const std::pair<subdomain_id_type, SubdomainName> & p) {
                               return p.second == findVal;
                             });
      if (it == original_subdomain_map.end())
        paramError("old_block_names",
                   "This parameter contains block names that do not exist in the input mesh.");
      else
        _old_block_ids.push_back((*it).first);
    }
  }

  MeshTools::Modification::rotate(*_meshes, 90.0, 0.0, 0.0);
  auto meshes_dup = _meshes->clone();
  _azimuthal_angle_meta = azimuthalAnglesCollector(
      dynamic_pointer_cast<ReplicatedMesh>(meshes_dup), -180.0, 180.0, ANGLE_DEGREE);
  // So that this class works for both derived classes of PolygonMeshGeneratorBase
  _pattern_pitch_meta = std::max(getMeshProperty<Real>("pitch_meta", _input_name),
                                 getMeshProperty<Real>("pattern_pitch_meta", _input_name));

  Real radiusCorrectionFactor_original =
      _preserve_volumes ? radiusCorrectionFactor(_azimuthal_angle_meta) : 1.0;

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

  angleIndentifier(_start_angle,
                   &original_start_down,
                   &original_start_down_it,
                   &original_start_up,
                   &original_start_up_it);

  Real original_end_down;
  std::vector<Real>::iterator original_end_down_it;
  Real original_end_up;
  std::vector<Real>::iterator original_end_up_it;

  angleIndentifier(
      _end_angle, &original_end_down, &original_end_down_it, &original_end_up, &original_end_up_it);

  Real azi_to_mod_start;
  Real azi_to_keep_start;

  angleModifier(side_angular_shift,
                side_angular_range,
                azi_tol,
                _start_angle,
                &original_start_down,
                &original_start_down_it,
                &original_start_up,
                &original_start_up_it,
                &azi_to_keep_start,
                &azi_to_mod_start);

  Real azi_to_mod_end;
  Real azi_to_keep_end;

  angleModifier(side_angular_shift,
                side_angular_range,
                azi_tol,
                _end_angle,
                &original_end_down,
                &original_end_down_it,
                &original_end_up,
                &original_end_up_it,
                &azi_to_keep_end,
                &azi_to_mod_end);

  if (azi_to_mod_end == azi_to_mod_start)
    paramError("angle_range",
               "The azimuthal intervals of the input mesh are too coarse for this parameter.");

  Real radiusCorrectionFactor_mod =
      _preserve_volumes ? radiusCorrectionFactor(_azimuthal_angle_meta) : 1.0;

  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      _meshes->get_boundary_info().build_side_list();

  // See if the block that contains the external boundary is modified or not
  bool external_block_change(false);
  for (unsigned int i = 0; i < side_list.size(); i++)
  {
    if (std::get<2>(side_list[i]) == OUTER_SIDESET_ID)
    {
      dof_id_type block_id_tmp = (*_meshes->elem_ptr(std::get<0>(side_list[i]))).subdomain_id();
      if (std::find(_old_block_ids.begin(), _old_block_ids.end(), block_id_tmp) !=
          _old_block_ids.end())
        external_block_change = true;
    }
  }

  _meshes->get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      _meshes->get_boundary_info().build_node_list();

  std::vector<std::pair<Real, dof_id_type>> node_id_keep_start;
  std::vector<std::pair<Real, dof_id_type>> node_id_mod_start;
  std::vector<std::pair<Real, dof_id_type>> node_id_keep_end;
  std::vector<std::pair<Real, dof_id_type>> node_id_mod_end;

  // Determine which nodes are involved in the elements that are intercepted by the staring/ending
  // angles
  for (libMesh::ReplicatedMesh::node_iterator node_it = _meshes->nodes_begin();
       node_it != _meshes->nodes_end();
       node_it++)
  {
    Point pt_tmp = *_meshes->node_ptr((*node_it)->id());
    Real node_azi = atan2(pt_tmp(1), pt_tmp(0)) * 180.0 / M_PI;
    Real node_rad = std::sqrt(pt_tmp(0) * pt_tmp(0) + pt_tmp(1) * pt_tmp(1));
    if (node_rad > rad_tol && (std::abs(node_azi - azi_to_mod_start) < azi_tol ||
                               std::abs(std::abs(node_azi - azi_to_mod_start) - 360.0) < azi_tol))
      node_id_mod_start.push_back(std::make_pair(node_rad, (*node_it)->id()));
    if (node_rad > rad_tol && (std::abs(node_azi - azi_to_keep_start) < azi_tol ||
                               std::abs(std::abs(node_azi - azi_to_keep_start) - 360.0) < azi_tol))
      node_id_keep_start.push_back(std::make_pair(node_rad, (*node_it)->id()));
    if (node_rad > rad_tol && (std::abs(node_azi - azi_to_mod_end) < azi_tol ||
                               std::abs(std::abs(node_azi - azi_to_mod_end) - 360.0) < azi_tol))
      node_id_mod_end.push_back(std::make_pair(node_rad, (*node_it)->id()));
    if (node_rad > rad_tol && (std::abs(node_azi - azi_to_keep_end) < azi_tol ||
                               std::abs(std::abs(node_azi - azi_to_keep_end) - 360.0) < azi_tol))
      node_id_keep_end.push_back(std::make_pair(node_rad, (*node_it)->id()));
  }
  // Sort the involved nodes using radius as the key; this facilitates the determination of circular
  // regions nodes
  std::sort(node_id_mod_start.begin(), node_id_mod_start.end());
  std::sort(node_id_keep_start.begin(), node_id_keep_start.end());
  std::sort(node_id_mod_end.begin(), node_id_mod_end.end());
  std::sort(node_id_keep_end.begin(), node_id_keep_end.end());
  std::vector<Real> circular_rad_list;
  std::vector<Real> non_circular_rad_list;

  _meshes = nodeModifier(dynamic_pointer_cast<ReplicatedMesh>(_meshes),
                         node_id_mod_start,
                         node_id_keep_start,
                         &circular_rad_list,
                         &non_circular_rad_list,
                         node_list,
                         _start_angle,
                         external_block_change,
                         rad_tol);
  _meshes = nodeModifier(dynamic_pointer_cast<ReplicatedMesh>(_meshes),
                         node_id_mod_end,
                         node_id_keep_end,
                         &circular_rad_list,
                         &non_circular_rad_list,
                         node_list,
                         _end_angle,
                         external_block_change,
                         rad_tol);

  const Real max_circular_radius =
      *std::max_element(circular_rad_list.begin(), circular_rad_list.end());
  const Real min_non_circular_radius =
      *std::min_element(non_circular_rad_list.begin(), non_circular_rad_list.end());

  // Re-correct Radii
  if (_preserve_volumes)
  {
    if (min_non_circular_radius <
        max_circular_radius / radiusCorrectionFactor_original * radiusCorrectionFactor_mod)
      mooseError("In AzimuthalBlockIDMeshGenerator ",
                 _name,
                 ": the circular region is overlapped with background region after correction.");
    for (libMesh::ReplicatedMesh::node_iterator node_it = _meshes->nodes_begin();
         node_it != _meshes->nodes_end();
         node_it++)
    {
      Point pt_tmp = *_meshes->node_ptr((*node_it)->id());
      Real node_rad = std::sqrt(pt_tmp(0) * pt_tmp(0) + pt_tmp(1) * pt_tmp(1));
      // Any nodes with radii smaller than the threshold belong to circular regions
      if (node_rad > rad_tol && node_rad <= max_circular_radius + rad_tol)
      {
        const Real node_azi = atan2(pt_tmp(1), pt_tmp(0));
        const Real node_rad_corr =
            node_rad / radiusCorrectionFactor_original * radiusCorrectionFactor_mod;
        pt_tmp(1) = node_rad_corr * std::sin(node_azi);
        pt_tmp(0) = node_rad_corr * std::cos(node_azi);
        _meshes->add_point(pt_tmp, (*node_it)->id());
      }
    }
  }
  // Assign New Block IDs
  for (unsigned int block_id_index = 0; block_id_index < _old_block_ids.size(); block_id_index++)
    for (libMesh::ReplicatedMesh::element_iterator elem_it =
             _meshes->active_subdomain_elements_begin(_old_block_ids[block_id_index]);
         elem_it != _meshes->active_subdomain_elements_end(_old_block_ids[block_id_index]);
         elem_it++)
    {
      auto p_cent = (*elem_it)->true_centroid();
      auto p_cent_azi = atan2(p_cent(1), p_cent(0)) * 180.0 / M_PI;
      if (_start_angle < _end_angle && p_cent_azi >= _start_angle && p_cent_azi <= _end_angle)
        (*elem_it)->subdomain_id() = _new_block_ids[block_id_index];
      else if (_start_angle > _end_angle &&
               (p_cent_azi >= _start_angle || p_cent_azi <= _end_angle))
        (*elem_it)->subdomain_id() = _new_block_ids[block_id_index];
    }
  // Assign new Block Names if applicable
  for (unsigned int i = 0; i < _new_block_names.size(); i++)
    _meshes->subdomain_name(_new_block_ids[i]) = _new_block_names[i];
  MeshTools::Modification::rotate(*_meshes, -90.0, 0.0, 0.0);
  for (unsigned int i = 0; i < _azimuthal_angle_meta.size(); i++)
    _azimuthal_angle_meta[i] = (_azimuthal_angle_meta[i] - 90.0 <= -180.0)
                                   ? (_azimuthal_angle_meta[i] + 270.0)
                                   : _azimuthal_angle_meta[i] - 90.0;
  std::sort(_azimuthal_angle_meta.begin(), _azimuthal_angle_meta.end());

  return dynamic_pointer_cast<MeshBase>(_meshes);
}

void
AzimuthalBlockIDMeshGenerator::angleIndentifier(
    const Real terminal_angle,
    Real * const original_down,
    std::vector<Real>::iterator * const original_down_it,
    Real * const original_up,
    std::vector<Real>::iterator * const original_up_it)
{

  auto term_up =
      std::lower_bound(_azimuthal_angle_meta.begin(), _azimuthal_angle_meta.end(), terminal_angle);
  if (term_up == _azimuthal_angle_meta.begin())
  {
    *original_up = *term_up;
    *original_up_it = term_up;
    *original_down = -180.0;
    *original_down_it = _azimuthal_angle_meta.end() - 1;
  }
  else if (term_up == _azimuthal_angle_meta.end())
  {
    *original_down = _azimuthal_angle_meta.back();
    *original_down_it = _azimuthal_angle_meta.end() - 1;
    *original_up = 180.0;
    *original_up_it = _azimuthal_angle_meta.begin();
  }
  else
  {
    *original_down = *(term_up - 1);
    *original_down_it = term_up - 1;
    *original_up = *term_up;
    *original_up_it = term_up;
  }
}

void
AzimuthalBlockIDMeshGenerator::angleModifier(const Real side_angular_shift,
                                             const Real side_angular_range,
                                             const Real azi_tol,
                                             const Real terminal_angle,
                                             Real * const original_down,
                                             std::vector<Real>::iterator * const original_down_it,
                                             Real * const original_up,
                                             std::vector<Real>::iterator * const original_up_it,
                                             Real * const azi_to_keep,
                                             Real * const azi_to_mod)
{
  const Real half_interval = (*original_up - *original_down) / 2.0;
  if (std::abs((*original_down + side_angular_shift) / side_angular_range -
               std::round((*original_down + side_angular_shift) / side_angular_range)) < azi_tol &&
      std::abs(*original_down - terminal_angle) > azi_tol)
  {
    *azi_to_keep = *original_down;
    *azi_to_mod = *original_up;
    *(*original_up_it) = terminal_angle;
  }
  else if (std::abs((*original_up + side_angular_shift) / side_angular_range -
                    std::round((*original_up + side_angular_shift) / side_angular_range)) <
               azi_tol &&
           std::abs(*original_up - terminal_angle) > azi_tol)
  {
    *azi_to_keep = *original_up;
    *azi_to_mod = *original_down;
    *(*original_down_it) = terminal_angle;
  }
  else if (terminal_angle - *original_down > half_interval)
  {
    *azi_to_keep = *original_down;
    *azi_to_mod = *original_up;
    *(*original_up_it) = terminal_angle;
  }
  else
  {
    *azi_to_keep = *original_up;
    *azi_to_mod = *original_down;
    *(*original_down_it) = terminal_angle;
  }
}

std::unique_ptr<ReplicatedMesh>
AzimuthalBlockIDMeshGenerator::nodeModifier(
    std::unique_ptr<ReplicatedMesh> mesh,
    const std::vector<std::pair<Real, dof_id_type>> node_id_mod,
    const std::vector<std::pair<Real, dof_id_type>> node_id_keep,
    std::vector<Real> * const circular_rad_list,
    std::vector<Real> * const non_circular_rad_list,
    const std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list,
    const Real term_angle,
    const bool external_block_change,
    const Real rad_tol)
{
  for (unsigned int i = 0; i < node_id_mod.size(); i++)
  {
    // Circular regions, radius is not alterned
    if (std::abs(node_id_mod[i].first - node_id_keep[i].first) < rad_tol)
    {
      Point pt_tmp = *mesh->node_ptr(node_id_mod[i].second);
      (*circular_rad_list).push_back(node_id_mod[i].first);
      pt_tmp(1) = node_id_mod[i].first * std::sin(term_angle * M_PI / 180.0);
      pt_tmp(0) = node_id_mod[i].first * std::cos(term_angle * M_PI / 180.0);
      mesh->add_point(pt_tmp, node_id_mod[i].second);
    }
    else
    {
      (*non_circular_rad_list).push_back(node_id_mod[i].first);
      // For external boundary nodes, if the external block is not modified, the boundary nodes are
      // not moved.
      // Non-circular range, use intercept instead
      if (std::find(node_list.begin(),
                    node_list.end(),
                    std::make_tuple(node_id_mod[i].second, OUTER_SIDESET_ID)) == node_list.end() ||
          external_block_change)
      {
        Point pt_1 = *mesh->node_ptr(node_id_mod[i].second);
        Point pt_2 = *mesh->node_ptr(node_id_keep[i].second);
        std::pair<Real, Real> pair_tmp = fourPointIntercept(
            std::make_pair(pt_1(0), pt_1(1)),
            std::make_pair(pt_2(0), pt_2(1)),
            std::make_pair(0.0, 0.0),
            std::make_pair(2.0 * node_id_mod[i].first * std::cos(term_angle * M_PI / 180.0),
                           2.0 * node_id_mod[i].first * std::sin(term_angle * M_PI / 180.0)));
        Point pt_tmp = *mesh->node_ptr(node_id_mod[i].second);
        pt_tmp(1) = pair_tmp.second;
        pt_tmp(0) = pair_tmp.first;
        mesh->add_point(pt_tmp, node_id_mod[i].second);
      }
    }
  }
  return mesh;
}
