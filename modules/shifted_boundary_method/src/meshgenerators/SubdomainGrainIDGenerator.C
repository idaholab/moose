//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainGrainIDGenerator.h"
#include "SBMUtils.h"

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainGrainIDGenerator);

InputParameters
SubdomainGrainIDGenerator::validParams()
{
  InputParameters params = SBMSubdomainGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("boundary_mesh",
                                             "The boundary mesh to use for identifying grain IDs");

  // --- Point-In-Polyhedron checks ---

  params.addParam<Point>(
      "ray_direction", Point(0, 0, 0), "The direction of the ray for in-out testing.");

  params.addParam<Real>("eps",
                        libMesh::TOLERANCE,
                        "Tolerance value used for intersection or surface proximity checks. "
                        "This parameter determines whether a point is considered on the geometry "
                        "or on the in/out sides of the geometry.");

  params.addRangeCheckedParam<unsigned int>(
      "leaf_max_size",
      10,
      "leaf_max_size > 0",
      "Maximum number of elements in a leaf node of the KD-tree.");

  params.addClassDescription(
      "Based on the boundary_mesh, which contains distinct grain boundaries as separate "
      "watertight regions, this generator assigns grain IDs (subdomain IDs) to the volume mesh. "
      "Even if the mesh does not perfectly align with the grain boundaries, each element is "
      "assigned the selected grain ID, or retains its input subdomain ID when no grain is "
      "selected.");

  return params;
}

SubdomainGrainIDGenerator::SubdomainGrainIDGenerator(const InputParameters & parameters)
  : SBMSubdomainGeneratorBase(parameters),
    _boundary_mesh(getMesh("boundary_mesh")),
    _ray_direction(getParam<Point>("ray_direction")),
    _eps(getParam<Real>("eps")),
    _leaf_max_size(getParam<unsigned int>("leaf_max_size"))
{
}

std::unique_ptr<libMesh::MeshBase>
SubdomainGrainIDGenerator::generate()
{
  // Take ownership of the input mesh (already cloned by getMesh()).
  std::unique_ptr<libMesh::MeshBase> mesh = std::move(_input);

  std::set<SubdomainID> boundary_subdomain_ids;
  // Bind a const reference so that we call the non-deprecated const subdomain_name() overload.
  const libMesh::MeshBase & boundary_mesh = *_boundary_mesh;
  boundary_mesh.subdomain_ids(boundary_subdomain_ids);
  for (const auto subdomain_id : boundary_subdomain_ids)
    mesh->set_subdomain_name(subdomain_id, boundary_mesh.subdomain_name(subdomain_id));

  // (a) read the boundary mesh to be our own data structure
  buildSubdomainGroupedData();

  // (b) build the point-containment checker for each subdomain
  buildInOutTesters();

  std::vector<const SubdomainCheckerEntry *> candidate_entries;
  candidate_entries.reserve(_checker_entries.size());

  std::vector<SBMUtils::SubdomainOccupancy> candidate_occupancies;
  candidate_occupancies.reserve(_checker_entries.size());

  for (const auto & elem : mesh->active_element_ptr_range() /*gen only run rank = 0*/)
  {
    const auto elem_bbox = elem->loose_bounding_box();
    candidate_entries.clear();
    candidate_occupancies.clear();

    for (const auto & entry : _checker_entries)
      if (boundingBoxesIntersect(elem_bbox, entry.bbox, _eps))
        candidate_entries.push_back(&entry);

    if (candidate_entries.empty())
      for (const auto & entry : _checker_entries)
        candidate_entries.push_back(&entry);

    for (const auto * entry : candidate_entries)
    {
      const auto * const checker = entry->checker;
      const auto is_in_domain = [checker](const Point & point)
      { return checker->sideness(point) != SurfaceGeometry::SurfaceSide::OUTSIDE; };
      candidate_occupancies.push_back(
          {entry->subdomain_id,
           SBMUtils::elementDomainOccupancy(*elem, _qrule_order, is_in_domain)});
    }

    const auto subdomain = SBMUtils::selectSubdomainFromOccupancies(
        candidate_occupancies, _intercepted_subdomain_policy, _lambda);
    if (subdomain)
      elem->subdomain_id() = *subdomain;
  }

  // Signal that the mesh has been modified and needs preparation.
  mesh->unset_is_prepared();
  return mesh;
}

void
SubdomainGrainIDGenerator::buildSubdomainGroupedData()
{
  struct SubdomainBuildBucket
  {
    SubdomainID subdomain_id;
    std::size_t count = 0;
    BoundingBox bbox;
    bool has_bbox = false;
    std::vector<Point> centroids;
    std::vector<std::unique_ptr<SurfaceElement>> boundary_elements;
  };

  _boundary_owned = std::move(_boundary_mesh);
  _centroids_by_subdomain.clear();
  _boundary_elements_by_subdomain.clear();
  _bboxes_by_subdomain.clear();

  libMesh::MeshBase * boundary_mesh = _boundary_owned.get();
  const auto estimated_subdomains = boundary_mesh->n_subdomains();

  std::unordered_map<SubdomainID, std::size_t> bucket_index_by_subdomain;
  bucket_index_by_subdomain.reserve(estimated_subdomains);

  std::vector<SubdomainBuildBucket> buckets;
  buckets.reserve(estimated_subdomains);

  auto get_or_create_bucket = [&](const SubdomainID sid) -> SubdomainBuildBucket &
  {
    const auto [it, inserted] = bucket_index_by_subdomain.emplace(sid, buckets.size());
    if (inserted)
      buckets.push_back({sid, 0, {}, false, {}, {}});
    return buckets[it->second];
  };

  for (const auto & elem : boundary_mesh->active_element_ptr_range())
  {
    const auto sid = elem->subdomain_id();
    auto & bucket = get_or_create_bucket(sid);
    ++bucket.count;

    const auto elem_bbox = elem->loose_bounding_box();
    if (!bucket.has_bbox)
    {
      bucket.bbox = elem_bbox;
      bucket.has_bbox = true;
    }
    else
      bucket.bbox.union_with(elem_bbox);
  }

  _centroids_by_subdomain.reserve(buckets.size());
  _boundary_elements_by_subdomain.reserve(buckets.size());
  _bboxes_by_subdomain.reserve(buckets.size());

  for (auto & bucket : buckets)
  {
    bucket.centroids.reserve(bucket.count);
    bucket.boundary_elements.reserve(bucket.count);
    _bboxes_by_subdomain.emplace(bucket.subdomain_id, bucket.bbox);
  }

  for (const auto & elem : boundary_mesh->active_element_ptr_range())
  {
    const auto sid = elem->subdomain_id();
    auto & bucket = buckets[bucket_index_by_subdomain.at(sid)];

    bucket.centroids.emplace_back(elem->vertex_average());

    std::unique_ptr<SurfaceElement> bnd_elem;
    if (elem->type() == EDGE2)
      bnd_elem = std::make_unique<SurfaceEdge2>(elem);
    else if (elem->type() == TRI3)
      bnd_elem = std::make_unique<SurfaceTri3>(elem);
    else
      mooseError("Unsupported element type in SubdomainGrainIDGenerator");

    bucket.boundary_elements.emplace_back(std::move(bnd_elem));
  }

  for (auto & bucket : buckets)
  {
    _centroids_by_subdomain.emplace(bucket.subdomain_id, std::move(bucket.centroids));
    _boundary_elements_by_subdomain.emplace(bucket.subdomain_id,
                                            std::move(bucket.boundary_elements));
  }
}

void
SubdomainGrainIDGenerator::buildInOutTesters()
{
  _subdomain_id_checkers.clear();
  _subdomain_id_checkers.reserve(_boundary_elements_by_subdomain.size());
  _checker_entries.clear();
  _checker_entries.reserve(_boundary_elements_by_subdomain.size());

  const SurfaceGeometry::RayDirectionOptions ray_options{
      _ray_direction.norm_sq() == 0 ? SurfaceGeometry::RayDirectionMode::AUTO_PCA
                                    : SurfaceGeometry::RayDirectionMode::USER_SPECIFIED,
      _ray_direction};

  for (const auto & [subdomain_id, elements] : _boundary_elements_by_subdomain)
  {
    auto checker = std::make_unique<AdaptiveRayContainmentCheck>(
        elements, _centroids_by_subdomain.at(subdomain_id), ray_options, _eps, _leaf_max_size);

    auto * checker_ptr = checker.get();
    _subdomain_id_checkers[subdomain_id] = std::move(checker);
    _checker_entries.push_back({subdomain_id, _bboxes_by_subdomain.at(subdomain_id), checker_ptr});
  }
}

bool
SubdomainGrainIDGenerator::boundingBoxesIntersect(const BoundingBox & lhs,
                                                  const BoundingBox & rhs,
                                                  const Real tolerance)
{
  const Point lhs_min = lhs.min();
  const Point lhs_max = lhs.max();
  const Point rhs_min = rhs.min();
  const Point rhs_max = rhs.max();

  for (const auto d : make_range(LIBMESH_DIM))
    if (lhs_max(d) + tolerance < rhs_min(d) || rhs_max(d) + tolerance < lhs_min(d))
      return false;

  return true;
}
