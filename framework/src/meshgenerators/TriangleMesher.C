//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriangleMesher.h"

#ifdef LIBMESH_HAVE_TRIANGLE
#include "libmesh/mesh_triangle_interface.h"
#include "libmesh/mesh_triangle_holes.h"
#endif

registerMooseObject("MooseApp", TriangleMesher);

template <>
InputParameters
validParams<TriangleMesher>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<std::vector<Real>>("points",
                                             "Points for the geometry that is to be triangulated");

  params.addParam<Real>("max_area", 0, "Maximum triangle area (default 0 means no restriction)");
  params.addParam<Real>("min_angle", 0, "Minimum triangle angle (default 0 means no restriction)");
  params.addParam<bool>("verbose", false, "Whether or not to let Triangle print");
  params.addParam<bool>("allow_adding_points_on_boundary",
                        true,
                        "Whether or not to allow adding points on boundaries");

  params.addParam<std::vector<unsigned int>>(
      "segments", "Explicitly specify segments otherwise using the implicit");
  params.addParam<std::vector<unsigned int>>("segment_subdivisions",
                                             "Subdivisions of all segments");
  params.addParam<std::vector<int>>("segment_markers", "Boundary markers of all segments");

  params.addParam<std::vector<Real>>("regions", "Points identifying a list of regions");
  params.addParam<std::vector<SubdomainID>>("region_subdomain_ids", "Subdomain IDs of regions");
  params.addParam<std::vector<Real>>(
      "region_max_areas", "Maximum triangle area for regions (default 0 means no restriction)");

  params.addParam<std::vector<Real>>("circular_hole_centers", "List of center of circular holes");
  params.addParam<std::vector<Real>>("circular_hole_radii", "Circular hole radii");
  params.addParam<std::vector<unsigned int>>("circular_hole_num_side_points",
                                             "Number of sides for the circular holes");
  params.addParam<std::vector<MeshGeneratorName>>("mesh_holes", "The mesh that creates holes");

  params.addParamNamesToGroup("max_area min_angle verbose allow_adding_points_on_boundary",
                              "Controls");
  params.addParamNamesToGroup("segments segment_subdivisions segment_markers", "Segments");
  params.addParamNamesToGroup("regions region_subdomain_ids region_max_areas", "Regions");
  params.addParamNamesToGroup(
      "circular_hole_centers circular_hole_radii circular_hole_num_side_points mesh_holes",
      "Holes");
  return params;
}

TriangleMesher::TriangleMesher(const InputParameters & parameters) : MeshGenerator(parameters)
{
#ifdef LIBMESH_HAVE_TRIANGLE
  if (isParamValid(("circular_hole_centers")))
  {
    if (!isParamValid("circular_hole_radii"))
      mooseError("circular_hole_radii must be given when circular_hole_centers is set");
    if (!isParamValid("circular_hole_num_side_points"))
      mooseError("circular_hole_num_side_points must be given when circular_hole_centers is set");

    // create circle holes
    auto & radius = getParam<std::vector<Real>>("circular_hole_radii");
    unsigned int nholes = radius.size();
    auto & centers = getParam<std::vector<Real>>("circular_hole_centers");
    if (centers.size() != nholes * 2)
      mooseError("'circular_hole_centers' must be in twice size of 'circular_hole_radii'");
    auto & npoints = getParam<std::vector<unsigned int>>("circular_hole_num_side_points");
    if (npoints.size() != nholes)
      mooseError(
          "'circular_hole_num_side_points' must be in the same size of 'circular_hole_radii'");
    for (unsigned int i = 0; i < nholes; ++i)
    {
      Point x(centers[i + i], centers[i + i + 1]);
      TriangleInterface::Hole * hole = new TriangleInterface::PolygonHole(x, radius[i], npoints[i]);
      _holes.push_back(hole);
    }
  }

  // obtain all regions
  if (isParamValid("regions"))
  {
    auto & regions = getParam<std::vector<Real>>("regions");
    unsigned int nregions = regions.size() / 2;
    if (regions.size() != nregions * 2)
      mooseError("Number of entries in 'regions' must be even");

    std::vector<SubdomainID> subdomain_ids(nregions, 0);
    if (isParamValid("region_subdomain_ids"))
    {
      subdomain_ids = getParam<std::vector<SubdomainID>>("region_subdomain_ids");
      if (subdomain_ids.size() != nregions)
        mooseError("'regions' must be in twice size of 'region_subdomain_ids'");
    }

    std::vector<Real> subdomain_areas(nregions, 0);
    if (isParamValid("region_max_areas"))
    {
      subdomain_areas = getParam<std::vector<Real>>("region_max_areas");
      if (subdomain_areas.size() != nregions)
        mooseError("'regions' must be in twice size of 'region_max_areas'");
      for (auto & area : subdomain_areas)
        if (area < 0)
          mooseError("Entries in 'region_max_areas' must be greater than or equal to 0");
    }

    for (unsigned int i = 0; i < nregions; ++i)
    {
      Point x(regions[i + i], regions[i + i + 1]);
      TriangleInterface::Region * region;
      if (subdomain_areas[i] == 0)
        region = new TriangleInterface::Region(x, subdomain_ids[i]);
      else
        region = new TriangleInterface::Region(x, subdomain_ids[i], subdomain_areas[i]);
      _regions.push_back(region);
    }
  }
#endif

  // create proper mesher dependency
  if (isParamValid("mesh_holes"))
  {
    auto input_meshes = getParam<std::vector<MeshGeneratorName>>("mesh_holes");
    for (auto & mesh_name : input_meshes)
      _mesh_ptrs.push_back(&getMeshByName(mesh_name));
  }
}

std::unique_ptr<MeshBase>
TriangleMesher::generate()
{
  auto mesh = _mesh->buildMeshBaseObject();

#ifdef LIBMESH_HAVE_TRIANGLE
  // create mesh holes
  if (isParamValid("mesh_holes"))
  {
    auto input_meshes = getParam<std::vector<MeshGeneratorName>>("mesh_holes");
    std::map<dof_id_type, std::vector<Point>> boundary_points;
    for (auto & mesh : _mesh_ptrs)
    {
      if ((*mesh)->mesh_dimension() != 2)
        mooseError("TriangleMesher can only accept two-dimensional meshes");
      std::unique_ptr<ReplicatedMesh> rmesh = dynamic_pointer_cast<ReplicatedMesh>(*mesh);
      auto bpoints = rmesh->get_boundary_points();
      for (auto & part : bpoints)
      {
        std::vector<unsigned int> seg_indices(1, 0);
        for (auto & points : part.second)
        {
          seg_indices.push_back(seg_indices.back() + points.size());
          for (auto & p : points)
            boundary_points[part.first].push_back(p);
        }

        Point interior_point = rmesh->elem_ptr(part.first)->centroid();
        TriangleInterface::Hole * hole = new TriangleInterface::ArbitraryHole(
            interior_point, boundary_points[part.first], seg_indices);
        _holes.push_back(hole);
      }
    }
  }

  // Clear out any data which may have been in the Mesh
  mesh->clear();

  // Make sure the new Mesh will be 2D
  mesh->set_mesh_dimension(2);

  // process input parameters
  auto & points = getParam<std::vector<Real>>("points");
  unsigned int npoints = points.size() / 2;
  if (npoints * 2 != points.size())
    mooseError("Number of values in 'points' must be even");

  std::vector<std::pair<unsigned int, unsigned int>> segments;
  std::vector<int> markers;

  /*
   * If segments parameter is given, we read it into segments and also read markers if they are
   * given in segment_markers. segment_subdivisions will be used to add more points on the segments.
   *
   * If segments parameter is not given, default segments will be used, i.e. point 0 is connected to
   * point 1 and point 1 is connected to point 2, etc, and the last point wrapps arround to connect
   * back to point 0. Segments subdivision is done by adding more points on segments respecting the
   * connection. Markers are also assigned acoordingly.
   *
   * We always give default markers so that we can assign boundaries with Triangle.
   */
  if (isParamValid("segments"))
  {
    // add points first
    for (unsigned int p = 0; p < npoints; ++p)
      mesh->add_point(Point(points[p + p], points[p + p + 1]));

    auto & segs = getParam<std::vector<unsigned int>>("segments");
    unsigned int nsegs = segs.size() / 2;
    if (nsegs * 2 != segs.size())
      mooseError("Number of entries in 'segments' must be even");
    for (unsigned int i = 0; i < nsegs; ++i)
    {
      if (segs[i * 2] >= npoints || segs[i * 2 + 1] >= npoints)
        mooseError("Point indices is out of bounds (>= number of segments, ",
                   npoints,
                   " here) in 'segments'");
      segments.push_back(std::pair<unsigned int, unsigned int>(segs[i * 2], segs[i * 2 + 1]));
    }

    if (isParamValid("segment_subdivisions"))
    {
      auto & div = getParam<std::vector<unsigned int>>("segment_subdivisions");
      if (div.size() != nsegs)
        mooseError("Number of segment subdivisions is not equal to number of boundary segments");

      for (unsigned int i = 0; i < nsegs; ++i)
      {
        Point x0(points[segments[i].first * 2], points[segments[i].first * 2 + 1]);
        Point x1(points[segments[i].second * 2], points[segments[i].second * 2 + 1]);
        for (unsigned int j = 1; j < div[i]; ++j)
          mesh->add_point(x0 + j * (x1 - x0) / div[i]);
      }
    }

    if (isParamValid("segment_markers"))
    {
      markers = getParam<std::vector<int>>("segment_markers");
      if (markers.size() != nsegs)
        mooseError("Number of boundary markers is not equal to number of boundary segments");
      for (auto & marker : markers)
        if (marker <= 1)
          mooseError("Markers need to be strictly greater than 1");
    }
    else
      // use 0 to let Triangle find all boundary sides (Triangle will set the boundary sides with
      // marker 1)
      markers.resize(nsegs, 0);
  }
  else
  {
    std::vector<unsigned int> div(npoints, 1);
    if (isParamValid("segment_subdivisions"))
    {
      div = getParam<std::vector<unsigned int>>("segment_subdivisions");
      if (div.size() != npoints)
        mooseError("Number of segment subdivisions is not equal to number of boundary segments");
    }

    std::vector<int> raw_markers;
    if (isParamValid("segment_markers"))
    {
      raw_markers = getParam<std::vector<int>>("segment_markers");
      if (raw_markers.size() != npoints)
        mooseError("Number of boundary markers is not equal to number of boundary segments");
      for (auto & marker : raw_markers)
        if (marker <= 1)
          mooseError("Markers need to be strictly greater than 1");
    }
    else
      // use 0 to let Triangle find all boundary sides (Triangle will set the boundary sides with
      // marker 1)
      raw_markers.resize(npoints, 0);

    // add points and process boundary markers
    for (unsigned int p = 0; p < npoints; ++p)
    {
      Point x0(points[p + p], points[p + p + 1]);
      Point x1;
      if (p == npoints - 1)
        x1 = Point(points[0], points[1]);
      else
        x1 = Point(points[p + p + 2], points[p + p + 3]);
      for (unsigned int i = 0; i < div[p]; ++i)
      {
        mesh->add_point(x0 + i * (x1 - x0) / div[p]);
        markers.push_back(raw_markers[p]);
      }
    }
  }

  // Construct the Triangle Interface object
  // Note: we are using the implicit segment from the enclosing points.
  TriangleInterface t(dynamic_cast<UnstructuredMesh &>(*mesh));

  // Set custom variables for the triangulation
  t.desired_area() = getParam<Real>("max_area");
  t.minimum_angle() = getParam<Real>("min_angle");
  t.triangulation_type() = TriangleInterface::PSLG;
  t.elem_type() = TRI3;

  // use Y command line switch to prohibit the insertion of Steiner points on the mesh enclosure
  // including the enclosures of all holes
  if (!getParam<bool>("allow_adding_points_on_boundary"))
    t.extra_flags() = "Y";

  t.quiet() = !getParam<bool>("verbose");

  if (!segments.empty())
    t.segments = segments;

  t.attach_boundary_marker(&markers);

  if (_holes.size() > 0)
    t.attach_hole_list(&_holes);

  if (_regions.size() > 0)
    t.attach_region_list(&_regions);

  // Triangulate!
  t.triangulate();

  for (auto & hole_ptr : _holes)
    delete hole_ptr;

  for (auto & reg_ptr : _regions)
    delete reg_ptr;
#else
  mooseError("Triangle is not available in libMesh!\nlibMesh needs to be configured with "
             "'--disable-strict-lgpl'");
#endif

  return dynamic_pointer_cast<MeshBase>(mesh);
}
