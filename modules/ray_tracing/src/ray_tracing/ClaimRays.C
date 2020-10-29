//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ClaimRays.h"

// Local includes
#include "RayTracingStudy.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/mesh_tools.h"

ClaimRays::ClaimRays(RayTracingStudy & study,
                     MooseMesh & mesh,
                     const std::vector<std::shared_ptr<Ray>> & rays,
                     std::vector<std::shared_ptr<Ray>> & local_rays,
                     const bool do_exchange)
  : _mesh(mesh),
    _comm(_mesh.getMesh().comm()),
    _pid(_comm.rank()),
    _do_exchange(do_exchange),
    _study(study),
    _rays(rays),
    _local_rays(local_rays)
{
}

void
ClaimRays::claim()
{
  preClaim();

  _local_rays.clear();

  // Grab the point locator
  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh.getMesh());
  _point_locator->enable_out_of_mesh_mode();

  // Exchange: filter Rays into processors that _may_ claim them
  std::unordered_map<processor_id_type, std::vector<std::shared_ptr<Ray>>> rays_to_send;
  if (_do_exchange)
    for (auto & ray : _rays)
      for (processor_id_type pid = 0; pid < _comm.size(); ++pid)
        if (_inflated_bboxes[pid].contains_point(ray->currentPoint()))
          rays_to_send[pid].push_back(ray);

  // Functor for possibly claiming a vector of Rays
  std::function<void(processor_id_type, const std::vector<std::shared_ptr<Ray>> &)> claim_functor =
      [&](processor_id_type /* pid */, const std::vector<std::shared_ptr<Ray>> & rays) {
        for (auto & ray : rays)
          possiblyClaim(ray);
      };

  // Send the relevant Rays to everyone and then claim
  if (_do_exchange)
    Parallel::push_parallel_packed_range(_comm, rays_to_send, &_study, claim_functor);
  // Already have the relevant Rays, just claim
  else
    claim_functor(_pid, _rays);

  postClaim();
}

void
ClaimRays::possiblyClaim(const std::shared_ptr<Ray> & ray)
{
  prePossiblyClaimRay(ray);

  const auto elem =
      claimPoint(ray->currentPoint(), ray->id(), (*_point_locator)(ray->currentPoint()));
  if (elem)
  {
    _local_rays.push_back(ray);
    postClaimRay(_local_rays.back(), elem);
  }
}

const Elem *
ClaimRays::claimPoint(const Point & point, const RayID id, const Elem * elem)
{
  if (elem)
  {
    // Looking for smallest (even ID Ray) or largest (odd ID Ray) elem id
    const bool smallest = id % 2 == 0;

    // Start with the element we found, as it is a valid candidate
    const Elem * extremum_elem = elem;

    // All point neighbors for this element
    mooseAssert(_elem_point_neighbors.count(elem->id()), "Not in point neighbor map");
    const auto & neighbors = _elem_point_neighbors.at(elem->id());

    // Find element that matches the extremum criteria
    for (const auto & neighbor : neighbors)
    {
      mooseAssert(neighbor->active(), "Inactive neighbor");

      if ((smallest && neighbor->id() < extremum_elem->id()) || // satisfies
          (!smallest && neighbor->id() > extremum_elem->id()))  // ...one of the id checks
        if (neighbor->contains_point(point))                    // and also contains the point
          extremum_elem = neighbor;
    }

    // Claim the object if we own the extremum elem
    if (extremum_elem->processor_id() == _pid)
    {
      mooseAssert(extremum_elem->active(), "Inactive element");
      return extremum_elem;
    }
  }

  return nullptr;
}

void
ClaimRays::postClaimRay(std::shared_ptr<Ray> & ray, const Elem * elem)
{
  mooseAssert(_mesh.queryElemPtr(elem->id()) == elem, "Mesh doesn't contain elem");
  mooseAssert(elem->active(), "Inactive element");

  // If the incoming side is set and is not incoming, or if it is not set at all, see
  // if we can find an incoming side that is valid.
  auto starting_incoming_side = RayTracingCommon::invalid_side;
  if (!(!ray->invalidCurrentIncomingSide() &&
        _study.sidePtrHelper(elem, ray->currentIncomingSide())
            ->contains_point(ray->currentPoint()) &&
        _study.sideIsIncoming(elem, ray->currentIncomingSide(), ray->direction(), /* tid = */ 0)))
    for (const auto s : elem->side_index_range())
      if (_study.sidePtrHelper(elem, s)->contains_point(ray->currentPoint()) &&
          _study.sideIsIncoming(elem, s, ray->direction(), /* tid = */ 0))
      {
        starting_incoming_side = s;
        break;
      }

  ray->setStart(ray->currentPoint(), elem, starting_incoming_side);
}

void
ClaimRays::init()
{
  buildBoundingBoxes();
  buildPointNeighbors();
}

void
ClaimRays::buildBoundingBoxes()
{
  // Local bounding box
  _bbox = MeshTools::create_local_bounding_box(_mesh.getMesh());
  _global_bbox = _bbox;

  // Gather the bounding boxes of all processors
  std::vector<std::pair<Point, Point>> bb_points = {static_cast<std::pair<Point, Point>>(_bbox)};
  _comm.allgather(bb_points, true);
  _inflated_bboxes.resize(_comm.size());
  for (processor_id_type pid = 0; pid < _comm.size(); ++pid)
  {
    BoundingBox pid_bbox = static_cast<BoundingBox>(bb_points[pid]);
    pid_bbox.scale(0.01);
    _inflated_bboxes[pid] = pid_bbox;
    _global_bbox.union_with(pid_bbox);
  }

  // Find intersecting (neighbor) bounding boxes
  _inflated_neighbor_bboxes.clear();
  for (processor_id_type pid = 0; pid < _comm.size(); ++pid)
  {
    // Skip this processor
    if (pid == _pid)
      continue;
    // Insert if the searched processor's bbox intersects my bbox
    const auto & pid_bbox = _inflated_bboxes[pid];
    if (_bbox.intersects(pid_bbox))
      _inflated_neighbor_bboxes.emplace_back(pid, pid_bbox);
  }
}

void
ClaimRays::buildPointNeighbors()
{
  _elem_point_neighbors.clear();
  const auto & node_to_elem_map = _mesh.nodeToElemMap();

  for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
  {
    auto & fill = _elem_point_neighbors[elem->id()];
    for (unsigned int v = 0; v < elem->n_vertices(); ++v)
    {
      const auto & node = elem->node_ptr(v);
      for (const auto & neighbor_id : node_to_elem_map.at(node->id()))
      {
        if (neighbor_id == elem->id())
          continue;

        const auto & neighbor = _mesh.elemPtr(neighbor_id);
        if (std::count(fill.begin(), fill.end(), neighbor) == 0)
          fill.emplace_back(neighbor);
      }
    }
  }
}
