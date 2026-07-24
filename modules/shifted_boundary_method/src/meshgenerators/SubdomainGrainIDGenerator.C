//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainGrainIDGenerator.h"
#include <algorithm>

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainGrainIDGenerator);

InputParameters
SubdomainGrainIDGenerator::validParams()
{
  InputParameters params = SBMSubdomainGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("boundary_mesh",
                                             "The boundary mesh to use for identifying grain IDs");

  // --- Point-In-Polyhedron checks ---

  params.addParam<bool>("brute_force",
                        false,
                        "If true, use brute force to check if the point is inside the geometry "
                        "by loop over every elements.");

  params.addParam<Point>(
      "ray_direction", Point(0, 0, 0), "The direction of the ray for in-out testing.");

  params.addParam<Real>("eps",
                        libMesh::TOLERANCE,
                        "Tolerance value used for intersection or surface proximity checks. "
                        "This parameter determines whether a point is considered on the geometry "
                        "or on the in/out sides of the geometry.");

  params.addParam<int>(
      "leaf_max_size", 10, "Maximum number of elements in a leaf node of the KD-tree.");

  params.addClassDescription(
      "Based on the boundary_mesh, which contains distinct grain boundaries as separate "
      "watertight regions, this generator assigns grain IDs (subdomain IDs) to the volume mesh. "
      "Even if the mesh does not perfectly align with the grain boundaries, each element is "
      "assigned the grain ID of the region that occupies the largest portion of the element.");

  return params;
}

SubdomainGrainIDGenerator::SubdomainGrainIDGenerator(const InputParameters & parameters)
  : SBMSubdomainGeneratorBase(parameters),
    _boundary_mesh(getMesh("boundary_mesh")),
    _ray_direction(getParam<Point>("ray_direction")),
    _brute_force(getParam<bool>("brute_force")),
    _eps(getParam<Real>("eps")),
    _leaf_max_size(getParam<int>("leaf_max_size"))
{
}

std::unique_ptr<libMesh::MeshBase>
SubdomainGrainIDGenerator::generate()
{
  // Take ownership of the input mesh (already cloned by getMesh()).
  std::unique_ptr<libMesh::MeshBase> mesh = std::move(_input);

  for (unsigned int i = 1; i <= _boundary_mesh->n_subdomains(); ++i)
    mesh->subdomain_name(i) = _boundary_mesh->subdomain_name(i);

  // (a) read the boundary mesh to be our own data structure
  buildSubdomainGroupedData();

  const Order qrule_order = _qrule_order;

  auto initFEBase = [&](const Elem * elem)
  {
    FEType fe_type(elem->default_order(), LAGRANGE);
    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    QGauss qrule(elem->dim(), qrule_order);
    fe->get_xyz(); // this is very important, otherwise the quadrature points are not
                   // initialized
    fe->get_JxW();
    fe->attach_quadrature_rule(&qrule);
    fe->reinit(elem);
    return fe;
  };

  // (b) build the PointInPolyhedronCheck for each subdomain
  buildInOutTesters();

  std::vector<const SubdomainCheckerEntry *> candidate_entries;
  candidate_entries.reserve(_checker_entries.size());

  struct CandidateInfo
  {
    const SubdomainCheckerEntry * entry;
    unsigned int num_inside_nodes;
  };

  std::vector<CandidateInfo> candidate_infos;
  candidate_infos.reserve(_checker_entries.size());

  std::vector<Real> remaining_weights;

  for (const auto & elem : mesh->active_element_ptr_range() /*gen only run rank = 0*/)
  {
    const auto elem_bbox = elem->loose_bounding_box();
    candidate_entries.clear();
    candidate_infos.clear();

    for (const auto & entry : _checker_entries)
      if (boundingBoxesIntersect(elem_bbox, entry.bbox, _eps))
        candidate_entries.push_back(&entry);

    if (candidate_entries.empty())
      for (const auto & entry : _checker_entries)
        candidate_entries.push_back(&entry);

    SubdomainID fully_inside_subdomain;
    SubdomainID best_intercepted_subdomain;
    Real best_active_area = -1.0;

    bool find_fully_inside = false;

    for (const auto * entry : candidate_entries)
    {
      auto * checker_ptr = entry->checker;
      unsigned int num_inside_nodes = 0;
      for (unsigned int i = 0; i < elem->n_nodes(); ++i)
        if (checker_ptr->sideness(elem->point(i)) != SurfaceSide::OUTSIDE)
          ++num_inside_nodes;

      if (num_inside_nodes == elem->n_nodes())
      {
        fully_inside_subdomain = entry->subdomain_id;
        find_fully_inside = true;
        break;
      }

      candidate_infos.push_back({entry, num_inside_nodes});
    }

    if (find_fully_inside)
    {
      elem->subdomain_id() = fully_inside_subdomain;
      continue;
    }

    std::sort(candidate_infos.begin(),
              candidate_infos.end(),
              [](const CandidateInfo & lhs, const CandidateInfo & rhs)
              { return lhs.num_inside_nodes > rhs.num_inside_nodes; });

    auto fe = initFEBase(elem);
    const auto & JxW = fe->get_JxW();
    const auto & q_points = fe->get_xyz();

    remaining_weights.assign(q_points.size() + 1, 0.0);
    for (int i = static_cast<int>(q_points.size()) - 1; i >= 0; --i)
      remaining_weights[i] = remaining_weights[i + 1] + JxW[i];

    const Real total_area = remaining_weights.front();

    for (const auto & candidate : candidate_infos)
    {
      auto * checker_ptr = candidate.entry->checker;
      Real active_area = 0.0;
      bool eliminated = false;

      for (unsigned int i = 0; i < q_points.size(); ++i)
      {
        if (checker_ptr->sideness(q_points[i]) != SurfaceSide::OUTSIDE)
          active_area += JxW[i];

        if (active_area + remaining_weights[i + 1] <= best_active_area)
        {
          eliminated = true;
          break;
        }
      }

      if (eliminated)
        continue;

      if (active_area > best_active_area)
      {
        best_active_area = active_area;
        best_intercepted_subdomain = candidate.entry->subdomain_id;

        if (best_active_area >= total_area)
          break;
      }
    }

    elem->subdomain_id() = best_intercepted_subdomain;

    // TODO: take care of fully outside case
  }

  // Signal that the mesh has been modified and needs preparation.
  mesh->set_isnt_prepared();
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
    std::vector<std::unique_ptr<SBMBndElementBase>> boundary_elements;
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
      buckets.push_back({sid});
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

    std::unique_ptr<SBMBndElementBase> bnd_elem;
    if (elem->type() == EDGE2)
      bnd_elem = std::make_unique<SBMBndEdge2>(elem);
    else if (elem->type() == TRI3)
      bnd_elem = std::make_unique<SBMBndTri3>(elem);
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

  for (const auto & [subdomain_id, elements] : _boundary_elements_by_subdomain)
  {
    auto checker =
        std::make_unique<PointInPolyhedronCheck>(elements,
                                                 _centroids_by_subdomain.at(subdomain_id),
                                                 _ray_direction,
                                                 _brute_force,
                                                 _eps,
                                                 _leaf_max_size);

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

  for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
    if (lhs_max(d) + tolerance < rhs_min(d) || rhs_max(d) + tolerance < lhs_min(d))
      return false;

  return true;
}
