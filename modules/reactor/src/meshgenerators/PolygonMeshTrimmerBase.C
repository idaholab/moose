//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonMeshTrimmerBase.h"
#include "MooseMeshUtils.h"
#include "MathUtils.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

InputParameters
PolygonMeshTrimmerBase::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh that needs to be trimmed.");
  params.addParam<BoundaryName>("peripheral_trimming_section_boundary",
                                "Boundary formed by peripheral trimming.");
  params.addRangeCheckedParam<short>(
      "center_trim_starting_index",
      "center_trim_starting_index>=0 & center_trim_starting_index<12",
      "Index of the starting center trimming position.");
  params.addRangeCheckedParam<short>("center_trim_ending_index",
                                     "center_trim_ending_index>=0 & center_trim_ending_index<12",
                                     "Index of the ending center trimming position.");
  params.addParam<BoundaryName>("center_trimming_section_boundary",
                                "Boundary formed by center trimming (external_boundary will be "
                                "assigned if this parameter is not provided).");
  params.addParam<BoundaryName>("external_boundary",
                                "External boundary of the input mesh prior to the trimming.");
  params.addParam<SubdomainName>(
      "tri_elem_subdomain_name_suffix",
      "trimmer_tri",
      "Suffix to the block name used for quad elements that are trimmed/converted into "
      "triangular elements to avert degenerate quad elements");
  params.addParam<subdomain_id_type>(
      "tri_elem_subdomain_shift",
      "Customized id shift to define subdomain ids of the converted triangular elements.");

  params.addParamNamesToGroup(
      "center_trim_starting_index center_trim_ending_index center_trimming_section_boundary",
      "Center Trimming");
  params.addParamNamesToGroup("peripheral_trimming_section_boundary", "Peripheral Trimming");
  params.addParamNamesToGroup("tri_elem_subdomain_name_suffix tri_elem_subdomain_shift",
                              "Trimmed Boundary Repair");

  params.addClassDescription("This PolygonMeshTrimmerBase is the base class for "
                             "CartesianMeshTrimmer and HexagonMeshTrimmer.");

  return params;
}

PolygonMeshTrimmerBase::PolygonMeshTrimmerBase(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _trim_peripheral_region(getParam<std::vector<unsigned short>>("trim_peripheral_region")),
    _peripheral_trimming_section_boundary(
        isParamValid("peripheral_trimming_section_boundary")
            ? getParam<BoundaryName>("peripheral_trimming_section_boundary")
            : BoundaryName()),
    _center_trimming_section_boundary(
        isParamValid("center_trimming_section_boundary")
            ? getParam<BoundaryName>("center_trimming_section_boundary")
            : BoundaryName()),
    _external_boundary_name(isParamValid("external_boundary")
                                ? getParam<BoundaryName>("external_boundary")
                                : BoundaryName()),
    _tri_elem_subdomain_name_suffix(getParam<SubdomainName>("tri_elem_subdomain_name_suffix")),
    _tri_elem_subdomain_shift(isParamValid("tri_elem_subdomain_shift")
                                  ? getParam<subdomain_id_type>("tri_elem_subdomain_shift")
                                  : Moose::INVALID_BLOCK_ID),
    _input(getMeshByName(_input_name))
{
  declareMeshProperty("pattern_pitch_meta", 0.0);
  declareMeshProperty("input_pitch_meta", 0.0);
  declareMeshProperty("is_control_drum_meta", false);
  if (std::accumulate(_trim_peripheral_region.begin(), _trim_peripheral_region.end(), 0) == 0 &&
      !_peripheral_trimming_section_boundary.empty())
    paramError("peripheral_trimming_section_boundary",
               "this input parameter is not used if peripheral trimming is not performed.");
}

std::unique_ptr<MeshBase>
PolygonMeshTrimmerBase::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;

  // Passing metadata
  if (hasMeshProperty<Real>("input_pitch_meta", _input_name))
    setMeshProperty("input_pitch_meta", getMeshProperty<Real>("input_pitch_meta", _input_name));
  if (hasMeshProperty<bool>("is_control_drum_meta", _input_name))
    setMeshProperty("is_control_drum_meta",
                    getMeshProperty<bool>("is_control_drum_meta", _input_name));

  const boundary_id_type external_boundary_id =
      _external_boundary_name.empty()
          ? (boundary_id_type)OUTER_SIDESET_ID
          : MooseMeshUtils::getBoundaryID(_external_boundary_name, mesh);
  if (external_boundary_id == libMesh::BoundaryInfo::invalid_id)
    paramError("external_boundary",
               "the provided external boundary does not exist in the input mesh.");

  std::set<subdomain_id_type> subdomain_ids_set;
  mesh.subdomain_ids(subdomain_ids_set);

  if (*max_element(_trim_peripheral_region.begin(), _trim_peripheral_region.end()))
  {
    const boundary_id_type peripheral_trimming_section_boundary_id =
        _peripheral_trimming_section_boundary.empty()
            ? external_boundary_id
            : (MooseMeshUtils::getBoundaryIDs(mesh, {_peripheral_trimming_section_boundary}, true))
                  .front();
    peripheralTrimmer(mesh,
                      _trim_peripheral_region,
                      external_boundary_id,
                      peripheral_trimming_section_boundary_id,
                      subdomain_ids_set);
    mesh.get_boundary_info().sideset_name(peripheral_trimming_section_boundary_id) =
        _peripheral_trimming_section_boundary;
  }
  else if (hasMeshProperty<Real>("pattern_pitch_meta", _input_name))
    setMeshProperty("pattern_pitch_meta", getMeshProperty<Real>("pattern_pitch_meta", _input_name));

  if (_center_trim_sector_number < _num_sides * 2)
  {
    const boundary_id_type center_trimming_section_boundary_id =
        _center_trimming_section_boundary.empty()
            ? external_boundary_id
            : (MooseMeshUtils::getBoundaryIDs(mesh, {_center_trimming_section_boundary}, true))
                  .front();
    centerTrimmer(mesh,
                  _num_sides,
                  _center_trim_sector_number,
                  _trimming_start_sector,
                  external_boundary_id,
                  center_trimming_section_boundary_id,
                  subdomain_ids_set);
    mesh.get_boundary_info().sideset_name(center_trimming_section_boundary_id) =
        _center_trimming_section_boundary;
  }

  if (quasiTriElementsFixer(mesh, subdomain_ids_set))
    mesh.prepare_for_use();

  return std::move(_input);
}

void
PolygonMeshTrimmerBase::centerTrimmer(ReplicatedMesh & mesh,
                                      const unsigned int num_sides,
                                      const unsigned int center_trim_sector_number,
                                      const unsigned int trimming_start_sector,
                                      const boundary_id_type external_boundary_id,
                                      const boundary_id_type center_trimming_section_boundary_id,
                                      const std::set<subdomain_id_type> subdomain_ids_set)
{
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;

  std::vector<std::vector<Real>> bdry_pars = {
      {std::cos((Real)trimming_start_sector * M_PI / (Real)num_sides),
       std::sin((Real)trimming_start_sector * M_PI / (Real)num_sides),
       0.0},
      {-std::cos((Real)(trimming_start_sector + center_trim_sector_number) * M_PI /
                 (Real)num_sides),
       -std::sin((Real)(trimming_start_sector + center_trim_sector_number) * M_PI /
                 (Real)num_sides),
       0.0}};

  for (unsigned int i = 0; i < bdry_pars.size(); i++)
    lineRemover(mesh,
                bdry_pars[i],
                block_id_to_remove,
                subdomain_ids_set,
                center_trimming_section_boundary_id,
                external_boundary_id);
}

void
PolygonMeshTrimmerBase::peripheralTrimmer(
    ReplicatedMesh & mesh,
    const std::vector<unsigned short> trim_peripheral_region,
    const boundary_id_type external_boundary_id,
    const boundary_id_type peripheral_trimming_section_boundary_id,
    const std::set<subdomain_id_type> subdomain_ids_set)
{
  const unsigned int num_sides = trim_peripheral_region.size();
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;

  const Real unit_length = getMeshProperty<Real>("input_pitch_meta", _input_name) /
                           (num_sides == 6 ? std::sqrt(3.0) : 2.0);
  // Add metadata to input
  const Real multiplier = ((Real)getMeshProperty<unsigned int>("pattern_size", _input_name) - 1.0) *
                          (num_sides == 6 ? 0.75 : 1.0);
  const Real ch_length = multiplier * unit_length;
  setMeshProperty("pattern_pitch_meta", ch_length * 2.0);

  std::vector<std::vector<Real>> bdry_pars;
  if (num_sides == 6)
    bdry_pars = {{1.0 / std::sqrt(3.0), 1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {-1.0 / std::sqrt(3.0), 1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {-1.0, 0.0, -ch_length},
                 {-1.0 / std::sqrt(3.0), -1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {1.0 / std::sqrt(3.0), -1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {1.0, 0.0, -ch_length}};
  else
    bdry_pars = {{1.0, 0.0, -ch_length},
                 {0.0, 1.0, -ch_length},
                 {-1.0, 0.0, -ch_length},
                 {0.0, -1.0, -ch_length}};

  for (unsigned int i = 0; i < bdry_pars.size(); i++)
    if (trim_peripheral_region[i])
      lineRemover(mesh,
                  bdry_pars[i],
                  block_id_to_remove,
                  subdomain_ids_set,
                  peripheral_trimming_section_boundary_id,
                  external_boundary_id,
                  true);
}

void
PolygonMeshTrimmerBase::lineRemover(ReplicatedMesh & mesh,
                                    const std::vector<Real> bdry_pars,
                                    const subdomain_id_type block_id_to_remove,
                                    const std::set<subdomain_id_type> subdomain_ids_set,
                                    const boundary_id_type trimming_section_boundary_id,
                                    const boundary_id_type external_boundary_id,
                                    const bool assign_ext_to_new,
                                    const bool side_to_remove)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  boundary_info.build_node_list_from_side_list();
  auto bdry_node_list = boundary_info.build_node_list();
  // Only select the boundaries_to_conform
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> slc_bdry_side_list;
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
    if (std::get<2>(bdry_side_list[i]) == external_boundary_id)
      slc_bdry_side_list.push_back(bdry_side_list[i]);

  // Assign block id for elements to be removed
  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    const auto p_x = (*elem_it)->vertex_average()(0);
    const auto p_y = (*elem_it)->vertex_average()(1);
    if (lineSideDeterminator(p_x, p_y, bdry_pars[0], bdry_pars[1], bdry_pars[2], side_to_remove))
      (*elem_it)->subdomain_id() = block_id_to_remove;
  }

  // Identify the nodes near the boundary
  std::vector<dof_id_type> node_list;
  for (auto elem_it = mesh.active_subdomain_set_elements_begin(subdomain_ids_set);
       elem_it != mesh.active_subdomain_set_elements_end(subdomain_ids_set);
       elem_it++)
  {
    for (unsigned int i = 0; i < (*elem_it)->n_sides(); i++)
    {
      if ((*elem_it)->neighbor_ptr(i) != nullptr)
        if ((*((*elem_it)->neighbor_ptr(i))).subdomain_id() == block_id_to_remove)
        {
          node_list.push_back((*elem_it)->side_ptr(i)->node_ptr(0)->id());
          node_list.push_back((*elem_it)->side_ptr(i)->node_ptr(1)->id());
          boundary_info.add_side(*elem_it, i, trimming_section_boundary_id);
          if (assign_ext_to_new && trimming_section_boundary_id != external_boundary_id)
            boundary_info.add_side(*elem_it, i, external_boundary_id);
        }
    }
  }
  const auto unique_it = std::unique(node_list.begin(), node_list.end());
  node_list.resize(std::distance(node_list.begin(), unique_it));
  // Mark those nodes that are on a boundary that requires conformality
  // If both nodes of a side are involved, we should only move one node
  std::vector<bool> node_list_flag(node_list.size(), false);
  std::vector<Point> node_list_point(node_list.size(), Point(0.0, 0.0, 0.0));
  for (unsigned int i = 0; i < slc_bdry_side_list.size(); i++)
  {
    dof_id_type side_id_0 = mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i]))
                                ->side_ptr(std::get<1>(slc_bdry_side_list[i]))
                                ->node_ptr(0)
                                ->id();
    dof_id_type side_id_1 = mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i]))
                                ->side_ptr(std::get<1>(slc_bdry_side_list[i]))
                                ->node_ptr(1)
                                ->id();
    // True means the bdry node is in the node list of the trimming interface
    bool side_id_0_in =
        !(std::find(node_list.begin(), node_list.end(), side_id_0) == node_list.end());
    bool side_id_1_in =
        !(std::find(node_list.begin(), node_list.end(), side_id_1) == node_list.end());

    // True means on the removal side
    bool side_node_0_remove = lineSideDeterminator((*mesh.node_ptr(side_id_0))(0),
                                                   (*mesh.node_ptr(side_id_0))(1),
                                                   bdry_pars[0],
                                                   bdry_pars[1],
                                                   bdry_pars[2],
                                                   side_to_remove);
    bool side_node_1_remove = lineSideDeterminator((*mesh.node_ptr(side_id_1))(0),
                                                   (*mesh.node_ptr(side_id_1))(1),
                                                   bdry_pars[0],
                                                   bdry_pars[1],
                                                   bdry_pars[2],
                                                   side_to_remove);
    // If both nodes of that side are involved in the trimming interface
    if (side_id_0_in && side_id_1_in)
      // The side needs to be removed from the sideset
      // The other node will be handled by other element's side
      boundary_info.remove_side(mesh.elem_ptr(std::get<0>(slc_bdry_side_list[i])),
                                std::get<1>(slc_bdry_side_list[i]),
                                std::get<2>(slc_bdry_side_list[i]));
    // Node 0 is on the removal side while node 1 is on the retaining side
    else if (side_id_0_in && (side_node_0_remove != side_node_1_remove))
    {
      node_list_flag[std::distance(
          node_list.begin(), std::find(node_list.begin(), node_list.end(), side_id_0))] = true;
      const Point p0 = *mesh.node_ptr(side_id_0);
      const Point p1 = *mesh.node_ptr(side_id_1);

      node_list_point[std::distance(node_list.begin(),
                                    std::find(node_list.begin(), node_list.end(), side_id_0))] =
          twoLineIntersection(bdry_pars[0],
                              bdry_pars[1],
                              bdry_pars[2],
                              p0(1) - p1(1),
                              p1(0) - p0(0),
                              -(p0(0) * (p0(1) - p1(1)) + (p1(0) - p0(0)) * p0(1)));
    }
    // Node 1 is on the removal side while node 0 is on the retaining side
    else if (side_id_1_in && (side_node_0_remove != side_node_1_remove))
    {
      node_list_flag[std::distance(
          node_list.begin(), std::find(node_list.begin(), node_list.end(), side_id_1))] = true;
      const Point p0 = *mesh.node_ptr(side_id_0);
      const Point p1 = *mesh.node_ptr(side_id_1);

      node_list_point[std::distance(node_list.begin(),
                                    std::find(node_list.begin(), node_list.end(), side_id_1))] =
          twoLineIntersection(bdry_pars[0],
                              bdry_pars[1],
                              bdry_pars[2],
                              p0(1) - p1(1),
                              p1(0) - p0(0),
                              -(p0(0) * (p0(1) - p1(1)) + (p1(0) - p0(0)) * p0(1)));
    }
  }

  // move nodes
  for (unsigned int i = 0; i < node_list.size(); i++)
  {
    // Only one node in trimmed region
    // This means the node is on both the trimming boundary and the original external boundary.
    // In order to keep the shape of the original external boundary, the node is moved along the
    // original external boundary.
    if (node_list_flag[i])
      *(mesh.node_ptr(node_list[i])) = node_list_point[i];
    // Two nodes in trimmed region, only one is moved
    // This means the node is in the middle of the trimming boundary.
    // Just move it along the normal direction of the trimming line.
    else
    {
      const Real x0 = (*(mesh.node_ptr(node_list[i])))(0);
      const Real y0 = (*(mesh.node_ptr(node_list[i])))(1);
      (*(mesh.node_ptr(node_list[i])))(0) =
          (bdry_pars[1] * (bdry_pars[1] * x0 - bdry_pars[0] * y0) - bdry_pars[0] * bdry_pars[2]) /
          (bdry_pars[0] * bdry_pars[0] + bdry_pars[1] * bdry_pars[1]);
      (*(mesh.node_ptr(node_list[i])))(1) =
          (bdry_pars[0] * (-bdry_pars[1] * x0 + bdry_pars[0] * y0) - bdry_pars[1] * bdry_pars[2]) /
          (bdry_pars[0] * bdry_pars[0] + bdry_pars[1] * bdry_pars[1]);
    }
  }

  // Delete the block
  for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
       elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
       elem_it++)
    mesh.delete_elem(*elem_it);
  mesh.contract();
  mesh.find_neighbors();
  // Delete zero volume elements
  for (auto elem_it = mesh.elements_begin(); elem_it != mesh.elements_end(); elem_it++)
  {
    if (MooseUtils::absoluteFuzzyEqual((*elem_it)->volume(), 0.0))
    {
      for (unsigned int i = 0; i < (*elem_it)->n_sides(); i++)
      {
        if ((*elem_it)->neighbor_ptr(i) != nullptr)
        {
          boundary_info.add_side((*elem_it)->neighbor_ptr(i),
                                 ((*elem_it)->neighbor_ptr(i))->which_neighbor_am_i(*elem_it),
                                 external_boundary_id);
          boundary_info.add_side((*elem_it)->neighbor_ptr(i),
                                 ((*elem_it)->neighbor_ptr(i))->which_neighbor_am_i(*elem_it),
                                 trimming_section_boundary_id);
        }
      }
      mesh.delete_elem(*elem_it);
    }
  }
  mesh.contract();
  mesh.prepare_for_use();
}

bool
PolygonMeshTrimmerBase::lineSideDeterminator(const Real px,
                                             const Real py,
                                             const Real param_1,
                                             const Real param_2,
                                             const Real param_3,
                                             const bool direction_param,
                                             const Real dis_tol)
{
  const Real tmp = px * param_1 + py * param_2 + param_3;
  return direction_param ? tmp >= dis_tol : tmp <= dis_tol;
}

Point
PolygonMeshTrimmerBase::twoLineIntersection(const Real param_11,
                                            const Real param_12,
                                            const Real param_13,
                                            const Real param_21,
                                            const Real param_22,
                                            const Real param_23)
{
  return Point(
      (param_12 * param_23 - param_22 * param_13) / (param_11 * param_22 - param_21 * param_12),
      (param_13 * param_21 - param_23 * param_11) / (param_11 * param_22 - param_21 * param_12),
      0.0);
}

bool
PolygonMeshTrimmerBase::quasiTriElementsFixer(ReplicatedMesh & mesh,
                                              const std::set<subdomain_id_type> subdomain_ids_set)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // Define the subdomain id shift for the new TRI3 element subdomain(s)
  const subdomain_id_type max_subdomain_id(*subdomain_ids_set.rbegin());
  const subdomain_id_type tri_subdomain_id_shift =
      _tri_elem_subdomain_shift == Moose::INVALID_BLOCK_ID ? max_subdomain_id
                                                           : _tri_elem_subdomain_shift;
  mooseAssert(std::numeric_limits<subdomain_id_type>::max() - max_subdomain_id >
                  tri_subdomain_id_shift,
              "The TRI elements subdomain id to be assigned may exceed the numeric limit.");
  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  std::vector<std::tuple<Elem *, unsigned int, bool>> bad_elems_rec;
  // Loop over all the active elements to find any degenerate QUAD elements (i.e., QUAD elements
  // with three collinear nodes).
  for (auto & elem : as_range(mesh.active_elements_begin(), mesh.active_elements_end()))
  {
    const auto elem_angles = vertex_angles(*elem);
    if (MooseUtils::absoluteFuzzyEqual(elem_angles.front().first, M_PI, 0.001))
      bad_elems_rec.push_back(std::make_tuple(elem, elem_angles.front().second, false));
  }
  std::set<subdomain_id_type> new_subdomain_ids;
  // Loop over all the identified degenerate QUAD elements
  for (const auto & bad_elem : bad_elems_rec)
  {
    std::vector<boundary_id_type> elem_bdry_container;
    // elems 2 and 3 are the neighboring elements of the degenerate element corresponding to the two
    // collinear sides.
    Elem * elem_0 = std::get<0>(bad_elem);
    Elem * elem_1 = elem_0->neighbor_ptr(std::get<1>(bad_elem));
    Elem * elem_2 = elem_0->neighbor_ptr((std::get<1>(bad_elem) - 1) % elem_0->n_vertices());
    // If the elems 2 and 3 do not exist, the two sides are on the external boundary formed by
    // trimming.
    if (elem_1 == nullptr && elem_2 == nullptr)
      mesh.get_boundary_info().boundary_ids(elem_0, std::get<1>(bad_elem), elem_bdry_container);
    else
      paramError("input", "The input mesh has degenerate quad element before trimming.");

    // Record subdomain id of the degenerate element
    auto elem_block_id = elem_0->subdomain_id();
    // Define the three of four nodes that will be used to generate the TRI element
    auto pt0 = elem_0->node_ptr((std::get<1>(bad_elem) + 1) % elem_0->n_vertices());
    auto pt1 = elem_0->node_ptr((std::get<1>(bad_elem) + 2) % elem_0->n_vertices());
    auto pt2 = elem_0->node_ptr((std::get<1>(bad_elem) + 3) % elem_0->n_vertices());
    // Record all the element extra integers of the degenerate element
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
      exist_extra_ids[j] = elem_0->get_extra_integer(j);
    // Delete the degenerate QUAD element
    mesh.delete_elem(elem_0);
    // Create the new TRI element
    Elem * elem_Tri3 = mesh.add_elem(new Tri3);
    elem_Tri3->set_node(0) = pt0;
    elem_Tri3->set_node(1) = pt1;
    elem_Tri3->set_node(2) = pt2;
    // Retain the boundary information
    for (auto bdry_id : elem_bdry_container)
      boundary_info.add_side(elem_Tri3, 2, bdry_id);
    // Assign subdomain id for the TRI element by shifting its original subdomain id
    elem_Tri3->subdomain_id() = elem_block_id + tri_subdomain_id_shift;
    new_subdomain_ids.emplace(elem_block_id + tri_subdomain_id_shift);
    // Retain element extra integers
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
      elem_Tri3->set_extra_integer(j, exist_extra_ids[j]);
  }
  // Assign subdomain names for the new TRI elements
  for (auto & nid : new_subdomain_ids)
  {
    const SubdomainName old_name = mesh.subdomain_name(nid - tri_subdomain_id_shift);
    if (MooseMeshUtils::getSubdomainID(
            (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                              : old_name) +
                "_" + _tri_elem_subdomain_name_suffix,
            mesh) != Moose::INVALID_BLOCK_ID)
      paramError("tri_elem_subdomain_name_suffix",
                 "The new subdomain name already exists in the mesh.");
    mesh.subdomain_name(nid) =
        (old_name.empty() ? (SubdomainName)(std::to_string(nid - tri_subdomain_id_shift))
                          : old_name) +
        "_" + _tri_elem_subdomain_name_suffix;
    mooseWarning("Degenerate QUAD elements have been converted into TRI elements with a new "
                 "subdomain name: " +
                 mesh.subdomain_name(nid) + ".");
  }
  return bad_elems_rec.size();
}

std::vector<std::pair<Real, unsigned int>>
PolygonMeshTrimmerBase::vertex_angles(Elem & elem) const
{
  std::vector<std::pair<Real, unsigned int>> angles;
  const unsigned int n_vertices = elem.n_vertices();

  for (unsigned int i = 0; i < n_vertices; i++)
  {
    Point v1 = (*elem.node_ptr((i - 1) % n_vertices) - *elem.node_ptr(i % n_vertices));
    Point v2 = (*elem.node_ptr((i + 1) % n_vertices) - *elem.node_ptr(i % n_vertices));
    Real tmp = v1 * v2 / v1.norm() / v2.norm();
    if (tmp > 1.0)
      tmp = 1.0;
    else if (tmp < -1.0)
      tmp = -1.0;
    angles.push_back(std::make_pair(acos(tmp), i));
  }
  std::sort(angles.begin(), angles.end(), std::greater<>());
  return angles;
}
