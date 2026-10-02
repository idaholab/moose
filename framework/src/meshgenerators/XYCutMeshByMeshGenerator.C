//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYCutMeshByMeshGenerator.h"
#include "MooseMeshXYMeshCuttingUtils.h"
#include "MooseMeshXYCuttingUtils.h"
#include "MooseMeshElementConversionUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", XYCutMeshByMeshGenerator);

InputParameters
XYCutMeshByMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input",
                                             "The 2D primary mesh that needs to be cut.");
  params.addRequiredParam<MeshGeneratorName>(
      "cutter",
      "The 2D mesh whose outer boundary defines the cut polyline. The outer boundary must form "
      "a single simple closed loop.");

  MooseEnum mode("REMOVE_INSIDE REMOVE_OUTSIDE KEEP_BOTH", "REMOVE_INSIDE");
  params.addParam<MooseEnum>(
      "mode",
      mode,
      "Which side(s) of the cutter polyline to keep. 'REMOVE_INSIDE' deletes the portion of the "
      "primary mesh inside the cutter polyline; 'REMOVE_OUTSIDE' keeps only the inside; "
      "'KEEP_BOTH' splits the primary mesh into two labeled subdomains along the cut.");

  MooseEnum cutting_type("CUT_ELEM_POLY CUT_ELEM_TRI", "CUT_ELEM_POLY");
  params.addParam<MooseEnum>(
      "cutting_type",
      cutting_type,
      "Method used to handle primary elements crossed by the cutter polyline. 'CUT_ELEM_POLY' "
      "replaces each crossed element with a single C0POLYGON. 'CUT_ELEM_TRI' additionally "
      "triangulates those polygons into TRI3.");

  params.addRequiredParam<boundary_id_type>(
      "new_boundary_id", "Boundary id assigned to the new cut interface.");

  params.addRangeCheckedParam<Real>(
      "snap_tol",
      0.0,
      "snap_tol>=0",
      "If > 0, primary mesh nodes within this distance of the cutter polyline are projected "
      "onto the polyline before clipping. Helps avoid sliver polygons. Vertices of the polyline "
      "take precedence over edges.");
  params.addParam<bool>(
      "snap_only_interior_nodes",
      true,
      "If true (default), only primary nodes not on the primary mesh's external boundary are "
      "eligible for snapping. Preserves the primary mesh outer boundary shape.");

  params.addParam<SubdomainName>(
      "poly_elem_subdomain_name_suffix",
      "cut_poly",
      "Suffix appended to original subdomain names for blocks that received C0POLYGON elements "
      "(CUT_ELEM_POLY).");
  params.addParam<SubdomainName>(
      "tri_elem_subdomain_name_suffix",
      "cut_tri",
      "Suffix appended to original subdomain names for blocks that received TRI3 elements "
      "(CUT_ELEM_TRI).");
  params.addParam<SubdomainName>(
      "kept_inside_subdomain_name_suffix",
      "inside",
      "(KEEP_BOTH only) Suffix for the inside portion of each original block.");
  params.addParam<SubdomainName>(
      "kept_outside_subdomain_name_suffix",
      "outside",
      "(KEEP_BOTH only) Suffix for the outside portion of each original block.");
  params.addParam<subdomain_id_type>(
      "poly_subdomain_id_shift",
      "Customized id shift to define subdomain ids of the new subdomains created by the cut. "
      "Auto-computed if omitted.");

  params.addParam<bool>(
      "improve_tri_elements",
      false,
      "(CUT_ELEM_TRI only) Whether to improve TRI3 element quality along the new cut boundary.");

  params.addClassDescription(
      "Cuts a 2D XY mesh using the outer boundary of another 2D mesh as the cut curve. The "
      "cutter polyline may be non-convex but must form a single simple closed loop.");

  return params;
}

XYCutMeshByMeshGenerator::XYCutMeshByMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _mode(getParam<MooseEnum>("mode").template getEnum<Mode>()),
    _cutting_type(getParam<MooseEnum>("cutting_type").template getEnum<CutType>()),
    _input_name(getParam<MeshGeneratorName>("input")),
    _cutter_name(getParam<MeshGeneratorName>("cutter")),
    _new_boundary_id(getParam<boundary_id_type>("new_boundary_id")),
    _snap_tol(getParam<Real>("snap_tol")),
    _snap_only_interior_nodes(getParam<bool>("snap_only_interior_nodes")),
    _poly_elem_subdomain_name_suffix(getParam<SubdomainName>("poly_elem_subdomain_name_suffix")),
    _tri_elem_subdomain_name_suffix(getParam<SubdomainName>("tri_elem_subdomain_name_suffix")),
    _kept_inside_subdomain_name_suffix(
        getParam<SubdomainName>("kept_inside_subdomain_name_suffix")),
    _kept_outside_subdomain_name_suffix(
        getParam<SubdomainName>("kept_outside_subdomain_name_suffix")),
    _poly_subdomain_id_shift(isParamValid("poly_subdomain_id_shift")
                                 ? getParam<subdomain_id_type>("poly_subdomain_id_shift")
                                 : Moose::INVALID_BLOCK_ID),
    _improve_tri_elements(getParam<bool>("improve_tri_elements")),
    _input(getMeshByName(_input_name)),
    _cutter(getMeshByName(_cutter_name))
{
  if (_input_name == _cutter_name)
    paramError("cutter", "This parameter must be different from 'input'.");
  if (_mode != Mode::KEEP_BOTH && _cutting_type == CutType::CUT_ELEM_POLY &&
      _poly_elem_subdomain_name_suffix.empty())
    paramError("poly_elem_subdomain_name_suffix", "Suffix must be non-empty.");
}

std::unique_ptr<MeshBase>
XYCutMeshByMeshGenerator::generate()
{
  // Cache elem dim metadata on both inputs
  if (!_input->preparation().has_cached_elem_data)
    _input->cache_elem_data();
  if (!_cutter->preparation().has_cached_elem_data)
    _cutter->cache_elem_data();

  // The MeshGenerator system requires us to take ownership of every input mesh. We return the
  // primary; the cutter is consumed and discarded.
  auto cutter_mesh = std::move(_cutter);

  // Both inputs must be 2D ReplicatedMeshes
  auto * input_rep = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!input_rep)
    paramError("input", "Input is not a replicated mesh, which is required.");
  if (*(input_rep->elem_dimensions().begin()) != 2 ||
      *(input_rep->elem_dimensions().rbegin()) != 2)
    paramError("input", "Only 2D meshes are supported.");

  auto * cutter_rep = dynamic_cast<ReplicatedMesh *>(cutter_mesh.get());
  if (!cutter_rep)
    paramError("cutter", "Cutter is not a replicated mesh, which is required.");
  if (*(cutter_rep->elem_dimensions().begin()) != 2 ||
      *(cutter_rep->elem_dimensions().rbegin()) != 2)
    paramError("cutter", "Only 2D meshes are supported.");

  ReplicatedMesh & primary = *input_rep;
  ReplicatedMesh & cutter = *cutter_rep;

  // Extract the cutter outer polyline (CCW)
  std::vector<Point> polyline;
  try
  {
    polyline = MooseMeshXYMeshCuttingUtils::extractClosedOuterPolyline(cutter);
  }
  catch (MooseException & e)
  {
    paramError("cutter", e.what());
  }

  // Allocate temporary block / boundary ids
  std::set<subdomain_id_type> subdomain_ids_set;
  primary.subdomain_ids(subdomain_ids_set);
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;
  const subdomain_id_type base_shift =
      _poly_subdomain_id_shift == Moose::INVALID_BLOCK_ID ? max_subdomain_id + 2
                                                          : _poly_subdomain_id_shift;
  // For KEEP_BOTH we need two non-overlapping shifts. Offset the inside shift by (max+1) so
  // [outside_shift, outside_shift + max] and [inside_shift, inside_shift + max] don't collide.
  const subdomain_id_type outside_shift = base_shift;
  const subdomain_id_type inside_shift =
      _mode == Mode::KEEP_BOTH ? base_shift + (max_subdomain_id + 1) : base_shift;

  const boundary_id_type new_boundary_id_tmp = MooseMeshUtils::getNextFreeBoundaryID(primary);

  // Build a PointLocator on the cutter mesh for inside/outside tests
  if (!cutter.is_prepared())
    cutter.prepare_for_use();
  auto cutter_locator_handle = cutter.sub_point_locator();
  libMesh::PointLocatorBase & cutter_locator = *cutter_locator_handle;
  cutter_locator.enable_out_of_mesh_mode();

  // Snap pre-pass
  if (_snap_tol > 0.0)
    MooseMeshXYMeshCuttingUtils::snapNodesToPolyline(
        primary, polyline, _snap_tol, _snap_only_interior_nodes);

  const Real geom_tol = _snap_tol > 0.0 ? _snap_tol : libMesh::TOLERANCE;

  // Sanity check: if any cutter polyline vertex lies strictly inside a primary element whose
  // vertex classification is not "mixed" (i.e., no vertex is strictly inside the cutter and
  // no vertex is strictly outside), the cutter polyline winds within a single primary element.
  // This includes (a) cutter fully inside one primary element and (b) cutter polyline dipping
  // into a primary element through a single edge. v1 supports only single straight chords per
  // primary element.
  {
    if (!primary.is_prepared())
      primary.prepare_for_use();
    auto primary_locator = primary.sub_point_locator();
    primary_locator->enable_out_of_mesh_mode();
    auto cutter_loc2 = cutter_mesh->sub_point_locator();
    cutter_loc2->enable_out_of_mesh_mode();
    auto classify_for_check = [&cutter_loc2](const Point & p) -> short
    { return (*cutter_loc2)(p) ? +1 : -1; };
    for (const auto & v : polyline)
    {
      const Elem * containing = (*primary_locator)(v);
      if (!containing)
        continue;
      // Check strictly interior (not within tol of any vertex or edge)
      bool on_boundary = false;
      const auto n_v_c = containing->n_vertices();
      for (const auto i : make_range(n_v_c))
      {
        if ((containing->point(i) - v).norm() <= geom_tol)
        {
          on_boundary = true;
          break;
        }
        const Point & a = containing->point(i);
        const Point & b = containing->point((i + 1) % n_v_c);
        const Point ab = b - a;
        const Real ab2 = ab(0) * ab(0) + ab(1) * ab(1);
        if (ab2 == 0.0)
          continue;
        Real t = ((v(0) - a(0)) * ab(0) + (v(1) - a(1)) * ab(1)) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const Real cx = a(0) + t * ab(0);
        const Real cy = a(1) + t * ab(1);
        if (std::hypot(v(0) - cx, v(1) - cy) <= geom_tol)
        {
          on_boundary = true;
          break;
        }
      }
      if (on_boundary)
        continue;
      // Strictly interior. Check the containing element has at least one vertex inside and one
      // outside the cutter (mixed classification).
      bool has_inside = false, has_outside = false;
      for (const auto i : make_range(n_v_c))
      {
        const short c = classify_for_check(containing->point(i));
        if (c > 0)
          has_inside = true;
        else if (c < 0)
          has_outside = true;
      }
      if (!has_inside || !has_outside)
        paramError("input",
                   "XYCutMeshByMeshGenerator: cutter polyline winds within primary element ",
                   containing->id(),
                   " (cutter vertex at (",
                   v(0),
                   ", ",
                   v(1),
                   ") is strictly interior to this element without crossing it). Refine the "
                   "primary mesh near the cutter, or simplify the cutter.");
    }
  }

  const auto cut_mode =
      _mode == Mode::REMOVE_INSIDE
          ? MooseMeshXYMeshCuttingUtils::CutMode::REMOVE_INSIDE
          : (_mode == Mode::REMOVE_OUTSIDE
                 ? MooseMeshXYMeshCuttingUtils::CutMode::REMOVE_OUTSIDE
                 : MooseMeshXYMeshCuttingUtils::CutMode::KEEP_BOTH);

  // Pick suffixes per mode. For non-KEEP_BOTH only the kept-side suffix is consulted by the
  // utility, so we can pass an empty for the unused side.
  const SubdomainName outside_suffix = _mode == Mode::KEEP_BOTH
                                           ? _kept_outside_subdomain_name_suffix
                                           : (_mode == Mode::REMOVE_INSIDE
                                                  ? _poly_elem_subdomain_name_suffix
                                                  : SubdomainName());
  const SubdomainName inside_suffix = _mode == Mode::KEEP_BOTH
                                          ? _kept_inside_subdomain_name_suffix
                                          : (_mode == Mode::REMOVE_OUTSIDE
                                                 ? _poly_elem_subdomain_name_suffix
                                                 : SubdomainName());

  try
  {
    MooseMeshXYMeshCuttingUtils::meshCutterRemoverCutElemPoly(primary,
                                                              polyline,
                                                              cutter_locator,
                                                              cut_mode,
                                                              outside_shift,
                                                              inside_shift,
                                                              outside_suffix,
                                                              inside_suffix,
                                                              block_id_to_remove,
                                                              new_boundary_id_tmp,
                                                              geom_tol);
  }
  catch (MooseException & e)
  {
    const std::string what = e.what();
    if (what.find("multi-component retained region") != std::string::npos)
      paramError("input",
                 std::string(e.what()) +
                     " Refine the primary mesh near the cutter, or simplify the cutter.");
    else if (what.find("subdomain name already exists") != std::string::npos)
    {
      if (_mode == Mode::KEEP_BOTH)
        paramError("kept_inside_subdomain_name_suffix", e.what());
      else
        paramError("poly_elem_subdomain_name_suffix", e.what());
    }
    else
      mooseError("In XYCutMeshByMeshGenerator: ", e.what());
  }

  // CUT_ELEM_TRI = POLY then split polygons into TRI3.
  // In KEEP_BOTH mode, also split QUAD4 in the new subdomains so each subdomain ends up
  // uniformly TRI3 (otherwise Exodus output fails on mixed-type blocks).
  if (_cutting_type == CutType::CUT_ELEM_TRI)
  {
    const auto bdry_side_list = primary.get_boundary_info().build_side_list();
    std::vector<dof_id_type> poly_elems;
    std::vector<dof_id_type> quad_elems_to_split;
    for (auto elem_it = primary.active_elements_begin();
         elem_it != primary.active_elements_end();
         elem_it++)
    {
      Elem * elem = *elem_it;
      if (elem->type() == libMesh::C0POLYGON)
        poly_elems.push_back(elem->id());
      else if (_mode == Mode::KEEP_BOTH && elem->type() == libMesh::QUAD4 &&
               (elem->subdomain_id() >= outside_shift))
        quad_elems_to_split.push_back(elem->id());
    }

    std::vector<dof_id_type> converted;
    for (const auto elem_id : poly_elems)
      MooseMeshElementConversionUtils::polygonElemSplitter(
          primary, bdry_side_list, elem_id, converted);
    for (const auto elem_id : quad_elems_to_split)
      MooseMeshXYCuttingUtils::quadElemSplitter(primary, elem_id, 0);

    // For non-KEEP_BOTH: rename "_<poly_suffix>" to "_<tri_suffix>" on touched subdomains.
    // For KEEP_BOTH: keep "_inside" / "_outside" suffix unchanged.
    if (_mode != Mode::KEEP_BOTH)
    {
      const subdomain_id_type kept_shift =
          _mode == Mode::REMOVE_INSIDE ? outside_shift : inside_shift;
      const std::string poly_suffix = "_" + (std::string)_poly_elem_subdomain_name_suffix;
      const std::string tri_suffix = "_" + (std::string)_tri_elem_subdomain_name_suffix;
      for (const auto & sub_id : subdomain_ids_set)
      {
        const subdomain_id_type new_sid = sub_id + kept_shift;
        if (!MooseMeshUtils::hasSubdomainID(primary, new_sid))
          continue;
        const std::string & current = primary.subdomain_name(new_sid);
        if (current.size() >= poly_suffix.size() &&
            current.compare(current.size() - poly_suffix.size(),
                            poly_suffix.size(),
                            poly_suffix) == 0)
          primary.subdomain_name(new_sid) =
              current.substr(0, current.size() - poly_suffix.size()) + tri_suffix;
      }
    }

    // Mark originals (polygons + split quads) for removal
    for (const auto elem_id : poly_elems)
      primary.elem_ptr(elem_id)->subdomain_id() = block_id_to_remove;
    for (const auto elem_id : quad_elems_to_split)
      primary.elem_ptr(elem_id)->subdomain_id() = block_id_to_remove;

    for (auto elem_it = primary.active_subdomain_elements_begin(block_id_to_remove);
         elem_it != primary.active_subdomain_elements_end(block_id_to_remove);
         elem_it++)
      primary.delete_elem(*elem_it);
    primary.contract();

    if (_improve_tri_elements && MooseMeshUtils::hasBoundaryID(primary, new_boundary_id_tmp))
      MooseMeshXYCuttingUtils::boundaryTriElemImprover(primary, new_boundary_id_tmp);
  }

  // Rename temp boundary id to user-specified id
  MeshTools::Modification::change_boundary_id(primary, new_boundary_id_tmp, _new_boundary_id);

  primary.prepare_for_use();
  return std::move(_input);
}
