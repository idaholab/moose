//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYDelaunayGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/int_range.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/unstructured_mesh.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", XYDelaunayGenerator);

InputParameters
XYDelaunayGenerator::validParams()
{
  InputParameters params = SurfaceDelaunayGeneratorBase::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");
  MooseEnum tri_elem_type("TRI3 TRI6 TRI7 DEFAULT", "DEFAULT");

  params.addRequiredParam<MeshGeneratorName>(
      "boundary",
      "The input MeshGenerator defining the output outer boundary and required Steiner points.");
  params.addParam<std::vector<BoundaryName>>(
      "input_boundary_names", "2D-input-mesh boundaries defining the output mesh outer boundary");
  params.addParam<std::vector<SubdomainName>>(
      "input_subdomain_names", "1D-input-mesh subdomains defining the output mesh outer boundary");
  params.addParam<unsigned int>("add_nodes_per_boundary_segment",
                                0,
                                "How many more nodes to add in each outer boundary segment.");
  params.addParam<bool>(
      "refine_boundary", true, "Whether to allow automatically refining the outer boundary.");

  params.addParam<SubdomainName>("output_subdomain_name",
                                 "Subdomain name to set on new triangles.");
  params.addParam<SubdomainID>("output_subdomain_id", "Subdomain id to set on new triangles.");

  params.addParam<BoundaryName>(
      "output_boundary",
      "Boundary name to set on new outer boundary.  Default ID: 0 if no hole meshes are stitched; "
      "or maximum boundary ID of all the stitched hole meshes + 1.");
  params.addParam<std::vector<BoundaryName>>(
      "hole_boundaries",
      "Boundary names to set on holes.  Default IDs are numbered up from 1 if no hole meshes are "
      "stitched; or from maximum boundary ID of all the stitched hole meshes + 2.");

  params.addParam<bool>(
      "verify_holes",
      true,
      "Verify holes do not intersect boundary or each other.  Asymptotically costly.");

  params.addParam<bool>("smooth_triangulation",
                        false,
                        "Whether to do Laplacian mesh smoothing on the generated triangles.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<std::vector<bool>>("refine_holes",
                                     std::vector<bool>(),
                                     "Whether to allow automatically refining each hole boundary.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area, or 0 to skip uniform refinement");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired area as a function of x,y; omit to skip non-uniform refinement");

  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<MooseEnum>(
      "tri_element_type", tri_elem_type, "Type of the triangular elements to be generated.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");
  params.addParam<std::vector<Point>>("interior_points",
                                      {},
                                      "Interior node locations, if no smoothing is used. Any point "
                                      "outside the surface will not be meshed.");
  params.addParam<std::vector<FileName>>(
      "interior_point_files", {}, "Text file(s) with the interior points, one per line");
  params.addClassDescription("Triangulates meshes within boundaries defined by input meshes.");

  params.addRangeCheckedParam<Real>("outer_boundary_layer_thickness",
                                    0,
                                    "outer_boundary_layer_thickness>=0",
                                    "Thickness of the outer boundary layer to be added.");
  params.addParam<unsigned int>(
      "outer_boundary_layer_num", 0, "Number of layers for the outer boundary layer.");
  params.addRangeCheckedParam<Real>(
      "outer_boundary_layer_bias",
      1.0,
      "outer_boundary_layer_bias>0",
      "Bias factor for the thickness of each layer in the outer boundary layer.");

  params.addRangeCheckedParam<std::vector<Real>>(
      "holes_boundary_layer_thickness",
      "holes_boundary_layer_thickness>=0",
      "Thickness of the hole boundary layers to be added.");
  params.addParam<std::vector<unsigned int>>("holes_boundary_layer_num",
                                             "Numbers of layers for the hole boundary layers.");
  params.addRangeCheckedParam<std::vector<Real>>(
      "holes_boundary_layer_bias",
      "holes_boundary_layer_bias>0",
      "Bias factors for the thickness of each layer in the hole boundary layers.");

  params.addParamNamesToGroup("interior_points interior_point_files",
                              "Mandatory mesh interior nodes");

  return params;
}

XYDelaunayGenerator::XYDelaunayGenerator(const InputParameters & parameters)
  : SurfaceDelaunayGeneratorBase(parameters),
    _bdy_ptr(getMesh("boundary")),
    _add_nodes_per_boundary_segment(getParam<unsigned int>("add_nodes_per_boundary_segment")),
    _refine_bdy(getParam<bool>("refine_boundary")),
    _output_subdomain_id(0),
    _smooth_tri(getParam<bool>("smooth_triangulation")),
    _verify_holes(getParam<bool>("verify_holes")),
    _hole_ptrs(getMeshes("holes")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _refine_holes(getParam<std::vector<bool>>("refine_holes")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _algorithm(parameters.get<MooseEnum>("algorithm")),
    _tri_elem_type(parameters.get<MooseEnum>("tri_element_type")),
    _verbose_stitching(parameters.get<bool>("verbose_stitching")),
    _interior_points(getParam<std::vector<Point>>("interior_points")),
    _outer_boundary_layer_thickness(getParam<Real>("outer_boundary_layer_thickness")),
    _outer_boundary_layer_num(getParam<unsigned int>("outer_boundary_layer_num")),
    _outer_boundary_layer_bias(getParam<Real>("outer_boundary_layer_bias")),
    _holes_boundary_layer_thickness(
        isParamValid("holes_boundary_layer_thickness")
            ? getParam<std::vector<Real>>("holes_boundary_layer_thickness")
            : std::vector<Real>()),
    _holes_boundary_layer_num(isParamValid("holes_boundary_layer_num")
                                  ? getParam<std::vector<unsigned int>>("holes_boundary_layer_num")
                                  : std::vector<unsigned int>()),
    _holes_boundary_layer_bias(isParamValid("holes_boundary_layer_bias")
                                   ? getParam<std::vector<Real>>("holes_boundary_layer_bias")
                                   : std::vector<Real>())
{
  if ((_desired_area > 0.0 && !_desired_area_func.empty()) ||
      (_desired_area > 0.0 && _use_auto_area_func) ||
      (!_desired_area_func.empty() && _use_auto_area_func))
    paramError("desired_area_func",
               "Only one of the three methods ('desired_area', 'desired_area_func', and "
               "'_use_auto_area_func') to set element area limit should be used.");

  if (!_use_auto_area_func)
    if (isParamSetByUser("auto_area_func_default_size") ||
        isParamSetByUser("auto_area_func_default_size_dist") ||
        isParamSetByUser("auto_area_function_num_points") ||
        isParamSetByUser("auto_area_function_power"))
      paramError("use_auto_area_func",
                 "If this parameter is set to false, the following parameters should not be set: "
                 "'auto_area_func_default_size', 'auto_area_func_default_size_dist', "
                 "'auto_area_function_num_points', 'auto_area_function_power'.");

  if (!_stitch_holes.empty() && _stitch_holes.size() != _hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  for (auto hole_i : index_range(_stitch_holes))
    if (_stitch_holes[hole_i] && (hole_i >= _refine_holes.size() || _refine_holes[hole_i]))
      paramError("refine_holes", "Disable auto refine of any hole boundary to be stitched.");

  if (isParamValid("hole_boundaries"))
  {
    auto & hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");
    if (hole_boundaries.size() != _hole_ptrs.size())
      paramError("hole_boundaries", "Need one hole_boundaries entry per hole, if specified.");
  }
  // Copied from MultiApp.C
  const auto & positions_files = getParam<std::vector<FileName>>("interior_point_files");
  for (const auto p_file_it : index_range(positions_files))
  {
    const std::string positions_file = positions_files[p_file_it];
    MooseUtils::DelimitedFileReader file(positions_file, &_communicator);
    file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
    file.read();

    const std::vector<Point> & data = file.getDataAsPoints();
    for (const auto & d : data)
      _interior_points.push_back(d);
  }
  bool has_duplicates =
      std::any_of(_interior_points.begin(),
                  _interior_points.end(),
                  [&](const Point & p)
                  { return std::count(_interior_points.begin(), _interior_points.end(), p) > 1; });
  if (has_duplicates)
    paramError("interior_points", "Duplicate points were found in the provided interior points.");

  if ((_outer_boundary_layer_thickness > 0 && _outer_boundary_layer_num == 0) ||
      (_outer_boundary_layer_thickness == 0 && _outer_boundary_layer_num > 0))
    paramError(
        "outer_boundary_layer_thickness",
        "this parameter must be set as non-zero along with a non-zero outer_boundary_layer_num.");

  if (_outer_boundary_layer_num > 0)
  {
    if (_add_nodes_per_boundary_segment)
      paramError("add_nodes_per_boundary_segment",
                 "Cannot add nodes per boundary segment when using an outer boundary layer.");
    if (_refine_bdy)
      paramError("refine_boundary", "Cannot refine boundary when using an outer boundary layer.");
  }

  if ((_holes_boundary_layer_thickness.size() && _holes_boundary_layer_num.size() &&
       _holes_boundary_layer_bias.size()) !=
      (_holes_boundary_layer_thickness.size() || _holes_boundary_layer_num.size() ||
       _holes_boundary_layer_bias.size()))
    paramError(
        "holes_boundary_layer_thickness, holes_boundary_layer_num, and holes_boundary_layer_bias",
        "All three parameters must be specified or not specified together.");
  if (_holes_boundary_layer_thickness.size() &&
      _holes_boundary_layer_thickness.size() != _hole_ptrs.size())
    paramError("holes_boundary_layer_thickness",
               "If specified, this parameter must have the same length as 'holes'.");
  if (_holes_boundary_layer_num.size() && _holes_boundary_layer_num.size() != _hole_ptrs.size())
    paramError("holes_boundary_layer_num",
               "If specified, this parameter must have the same length as 'holes'.");
  if (_holes_boundary_layer_bias.size() && _holes_boundary_layer_bias.size() != _hole_ptrs.size())
    paramError("holes_boundary_layer_bias",
               "If specified, this parameter must have the same length as 'holes'.");
  for (const auto & i : index_range(_holes_boundary_layer_thickness))
  {
    if ((_holes_boundary_layer_thickness[i] > 0 && _holes_boundary_layer_num[i] == 0) ||
        (_holes_boundary_layer_thickness[i] == 0 && _holes_boundary_layer_num[i] > 0))
      paramError(
          "holes_boundary_layer_thickness",
          "entry " + std::to_string(i) +
              " must be set as non-zero along with a non-zero holes_boundary_layer_num entry.");
    if (_holes_boundary_layer_thickness[i] > 0)
    {
      if ((_refine_holes.size() > i && _refine_holes[i]) || _refine_holes.empty())
        paramError("refine_holes", "Cannot refine hole boundary when using a hole boundary layer.");
    }
  }
}

std::unique_ptr<MeshBase>
XYDelaunayGenerator::generate()
{
  MooseMeshUtils::XYDelaunayOptions xyd_opts;
  if (isParamValid("input_boundary_names"))
    xyd_opts.input_boundary_names = getParam<std::vector<BoundaryName>>("input_boundary_names");
  if (isParamValid("input_subdomain_names"))
    xyd_opts.input_subdomain_names = getParam<std::vector<SubdomainName>>("input_subdomain_names");
  xyd_opts.add_nodes_per_boundary_segment = _add_nodes_per_boundary_segment;
  xyd_opts.refine_bdy = _refine_bdy;
  xyd_opts.verify_holes = _verify_holes;
  xyd_opts.smooth_tri = _smooth_tri;
  xyd_opts.desired_area = _desired_area;
  xyd_opts.desired_area_func = _desired_area_func;
  xyd_opts.use_auto_area_func = _use_auto_area_func;
  xyd_opts.auto_area_func_default_size = _auto_area_func_default_size;
  xyd_opts.auto_area_func_default_size_dist = _auto_area_func_default_size_dist;
  xyd_opts.auto_area_function_num_points = _auto_area_function_num_points;
  xyd_opts.auto_area_function_power = _auto_area_function_power;
  xyd_opts.interior_points = _interior_points;
  xyd_opts.tri_elem_type = std::string(_tri_elem_type);
  xyd_opts.stitch_holes = _stitch_holes;
  xyd_opts.refine_holes = _refine_holes;
  xyd_opts.use_binary_search = (_algorithm == "BINARY");
  xyd_opts.verbose_stitching = _verbose_stitching;
  if (isParamValid("output_subdomain_id"))
  {
    xyd_opts.has_output_subdomain_id = true;
    xyd_opts.output_subdomain_id = getParam<SubdomainID>("output_subdomain_id");
  }
  if (isParamValid("output_subdomain_name"))
  {
    xyd_opts.has_output_subdomain_name = true;
    xyd_opts.output_subdomain_name = getParam<SubdomainName>("output_subdomain_name");
  }
  if (isParamValid("output_boundary"))
  {
    xyd_opts.has_output_boundary = true;
    xyd_opts.output_boundary = getParam<BoundaryName>("output_boundary");
  }
  if (isParamValid("hole_boundaries"))
    xyd_opts.hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");

  std::vector<std::unique_ptr<MeshBase>> hole_meshes(_hole_ptrs.size());
  for (auto hole_i : index_range(_hole_ptrs))
    hole_meshes[hole_i] = std::move(*_hole_ptrs[hole_i]);

  std::unique_ptr<MeshBase> boundary_mesh = std::move(_bdy_ptr);

  // Preserve the user-facing output_boundary so we can restore it after the outer-ring stitch-back
  // (we'll temporarily replace it with a sentinel name to locate the seam).
  const bool user_has_output_boundary = xyd_opts.has_output_boundary;
  const BoundaryName user_output_boundary = xyd_opts.output_boundary;
  const BoundaryName tmp_outer_name("__xyd_bdry_layer_tmp_outer__");

  const bool using_outer_layer = (_outer_boundary_layer_num > 0);
  const SubdomainID layer_sd_id =
      xyd_opts.has_output_subdomain_id ? xyd_opts.output_subdomain_id : SubdomainID(0);
  const SubdomainName layer_sd_name =
      xyd_opts.has_output_subdomain_name ? xyd_opts.output_subdomain_name : SubdomainName();

  // Build outer boundary-layer ring if requested. The ring's innermost side (bcid 1) becomes the
  // outer constraint for the interior triangulation; we then stitch a clone of the ring back onto
  // the result at that seam.
  std::unique_ptr<MeshBase> outer_ring_clone;
  if (using_outer_layer)
  {
    auto outer_ring = MooseMeshUtils::buildBoundaryLayerRing(*this,
                                                             *boundary_mesh,
                                                             xyd_opts.input_boundary_names,
                                                             _outer_boundary_layer_num,
                                                             _outer_boundary_layer_thickness,
                                                             _outer_boundary_layer_bias,
                                                             /*outward=*/false,
                                                             _tri_elem_type,
                                                             layer_sd_id,
                                                             layer_sd_name);
    outer_ring_clone = outer_ring->clone();
    boundary_mesh = std::move(outer_ring);
    xyd_opts.input_boundary_names = {BoundaryName("1")};
    xyd_opts.has_output_boundary = true;
    xyd_opts.output_boundary = tmp_outer_name;
  }

  // Build hole boundary-layer rings if requested. Each ring is stitched into the result by the
  // standard hole-stitching path in triangulateWithDelaunay (with stitch_holes forced true).
  for (auto hole_i : index_range(_hole_ptrs))
  {
    if (hole_i < _holes_boundary_layer_num.size() && _holes_boundary_layer_num[hole_i] > 0)
    {
      const bool keep_input = (hole_i < _stitch_holes.size() && _stitch_holes[hole_i]);
      auto hole_ring =
          MooseMeshUtils::buildBoundaryLayerRing(*this,
                                                 *hole_meshes[hole_i],
                                                 std::vector<BoundaryName>(),
                                                 _holes_boundary_layer_num[hole_i],
                                                 _holes_boundary_layer_thickness[hole_i],
                                                 _holes_boundary_layer_bias[hole_i],
                                                 /*outward=*/true,
                                                 _tri_elem_type,
                                                 layer_sd_id,
                                                 layer_sd_name);

      if (keep_input)
      {
        // Stitch the original hole mesh into the ring at ring's innermost side (bcid 1).
        auto & ring_u = dynamic_cast<UnstructuredMesh &>(*hole_ring);
        auto & inp_u = dynamic_cast<UnstructuredMesh &>(*hole_meshes[hole_i]);
        libMesh::MeshSerializer s1(ring_u), s2(inp_u);
        if (!ring_u.is_prepared())
          ring_u.prepare_for_use();
        if (!inp_u.is_prepared())
          inp_u.prepare_for_use();
        // Renumber input mesh boundary ids so they don't overlap with the ring's, mirroring the
        // approach in StitchMeshGenerator. This avoids degenerate stitching when both meshes
        // contain the same bcids on different sides.
        const auto & ring_bids = ring_u.get_boundary_info().get_global_boundary_ids();
        const auto inp_bids = inp_u.get_boundary_info().get_global_boundary_ids();
        BoundaryID ext_id = 1;
        bool overlap = false;
        for (auto b : inp_bids)
          if (ring_bids.count(b))
            overlap = true;
        if (overlap)
        {
          const auto max_bid = std::max(
              *ring_bids.rbegin(), inp_bids.empty() ? boundary_id_type(0) : *inp_bids.rbegin());
          BoundaryID idx = 1;
          for (auto b : inp_bids)
          {
            const auto new_b = max_bid + (idx++);
            inp_u.get_boundary_info().renumber_id(b, new_b);
          }
          ext_id = max_bid + idx;
        }
        else
          ext_id = MooseMeshUtils::getNextFreeBoundaryID(inp_u);
        inp_u.comm().max(ext_id);
        bool has_ext = false;
        MooseMeshUtils::addExternalBoundary(inp_u, ext_id, has_ext);
        if (has_ext)
        {
          const auto ring_u_ext_id =
              std::max(MooseMeshUtils::getNextFreeBoundaryID(ring_u), BoundaryID(ext_id + 1));
          MooseMeshUtils::changeBoundaryId(ring_u, 1, ring_u_ext_id, false);
          if (xyd_opts.hole_boundary_inner_id_defaults.size() <= hole_i)
            xyd_opts.hole_boundary_inner_id_defaults.resize(_hole_ptrs.size());
          xyd_opts.hole_boundary_inner_id_defaults[hole_i] = {ring_u_ext_id};
          // we want to keep the ring's original inner bcid (1) for later use.
          ring_u.stitch_meshes(inp_u,
                               1,
                               ext_id,
                               TOLERANCE,
                               /*clear_stitched_bcids=*/true,
                               _verbose_stitching,
                               _algorithm == "BINARY",
                               /*enforce_all_nodes_match_on_boundaries=*/false,
                               /*merge_boundary_nodes_all_or_nothing=*/false,
                               /*remap_subdomain_ids=*/false);
        }
      }

      hole_meshes[hole_i] = std::move(hole_ring);
      if (xyd_opts.stitch_holes.size() <= hole_i)
        xyd_opts.stitch_holes.resize(_hole_ptrs.size(), false);
      xyd_opts.stitch_holes[hole_i] = true;
      // The ring presents multiple external manifolds; tell triangulateWithDelaunay to use the
      // outermost (bcid (num_layers - 1) * 2) as the hole's outer boundary.
      if (xyd_opts.hole_boundary_id_filters.size() <= hole_i)
        xyd_opts.hole_boundary_id_filters.resize(_hole_ptrs.size());
      xyd_opts.hole_boundary_id_filters[hole_i] = {
          std::size_t((_holes_boundary_layer_num[hole_i] - 1) * 2)};
    }
  }

  auto result = MooseMeshUtils::triangulateWithDelaunay(
      *this, std::move(boundary_mesh), std::move(hole_meshes), xyd_opts);

  // Stitch the outer ring clone back onto the interior triangulation at the recorded seam.
  if (outer_ring_clone)
  {
    auto sentinel_ids = MooseMeshUtils::getBoundaryIDs(*result, {tmp_outer_name}, false);
    const boundary_id_type sentinel_id = sentinel_ids[0];

    // Preserve the ring's outermost bcid (= (num_layers - 1) * 2) by renaming it to a high temp
    // value so the post-stitch rename can recover it as the final outer bcid.
    const boundary_id_type ring_outermost_orig =
        boundary_id_type((_outer_boundary_layer_num - 1) * 2);
    const boundary_id_type ring_outermost_temp = 10000;
    libMesh::MeshTools::Modification::change_boundary_id(
        *outer_ring_clone, ring_outermost_orig, ring_outermost_temp);

    auto & result_u = dynamic_cast<UnstructuredMesh &>(*result);
    auto & clone_u = dynamic_cast<UnstructuredMesh &>(*outer_ring_clone);
    libMesh::MeshSerializer s1(result_u), s2(clone_u);
    result_u.stitch_meshes(clone_u,
                           sentinel_id,
                           1,
                           TOLERANCE,
                           /*clear_stitched_bcids=*/true,
                           _verbose_stitching,
                           _algorithm == "BINARY");

    libMesh::MeshTools::Modification::change_boundary_id(*result, ring_outermost_temp, sentinel_id);

    if (user_has_output_boundary)
    {
      auto user_id = MooseMeshUtils::getBoundaryIDs(*result, {user_output_boundary}, true).front();
      if (user_id != sentinel_id)
        libMesh::MeshTools::Modification::change_boundary_id(*result, sentinel_id, user_id);
      result->get_boundary_info().sideset_name(user_id) = user_output_boundary;
    }

    result->unset_is_prepared();
  }

  return result;
}
