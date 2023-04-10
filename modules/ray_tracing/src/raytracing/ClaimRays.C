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
                     const std::vector<std::shared_ptr<Ray>> & rays,
                     std::vector<std::shared_ptr<Ray>> & local_rays,
                     const bool do_exchange)
  : ParallelObject(study.comm()),
    MeshChangedInterface(study.parameters()),
    _mesh(study.mesh()),
    _pid(comm().rank()),
    _do_exchange(do_exchange),
    _study(study),
    _parallel_study(*_study.parallelStudy()),
    _rays(rays),
    _local_rays(local_rays),
    _needs_init(true)
{
}

void
ClaimRays::claim()
{
  if (_needs_init)
  {
    init();
    _needs_init = false;
  }

  preClaim();

  // Clear these as we're about to fill
  _local_rays.clear();

  // Grab the point locator
  _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, _mesh.getMesh());
  _point_locator->enable_out_of_mesh_mode();

  // Exchange: filter Rays into processors that _may_ claim them
  std::unordered_map<processor_id_type, std::vector<std::shared_ptr<Ray>>> rays_to_send;
  if (_do_exchange)
    for (processor_id_type pid = 0; pid < comm().size(); ++pid)
      if (_pid != pid)
      {
        const BoundingBox & pid_bbox = inflatedBoundingBox(pid);
        for (auto & ray : _rays)
          if (pid_bbox.contains_point(ray->currentPoint()))
            rays_to_send[pid].push_back(ray);
      }

  // Functor for possibly claiming a vector of Rays
  auto claim_functor =
      [&](processor_id_type /* pid */, const std::vector<std::shared_ptr<Ray>> & rays)
  {
    for (auto & ray : rays)
      possiblyClaim(ray);
  };

  // Send the relevant Rays to everyone and then attempt to claim the ones that we receive
  if (_do_exchange)
    Parallel::push_parallel_packed_range(comm(), rays_to_send, &_parallel_study, claim_functor);

  // Attempt to claim the locally generated rays in _rays
  claim_functor(_pid, _rays);

  // Verify the claiming if the study so desires
  if (_study.verifyRays())
    verifyClaiming();

  postClaim();
}

void
ClaimRays::possiblyClaim(const std::shared_ptr<Ray> & ray)
{
  prePossiblyClaimRay(ray);

  const auto elem =
      claimPoint(ray->currentPoint(), getID(ray), (*_point_locator)(ray->currentPoint()));
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
        _study.elemSide(*elem, ray->currentIncomingSide()).contains_point(ray->currentPoint()) &&
        _study.sideIsIncoming(elem, ray->currentIncomingSide(), ray->direction(), /* tid = */ 0)))
    for (const auto s : elem->side_index_range())
      if (_study.elemSide(*elem, s).contains_point(ray->currentPoint()) &&
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
ClaimRays::meshChanged()
{
  _needs_init = true;
}

void
ClaimRays::buildBoundingBoxes()
{
  // Local bounding box
  const auto bbox = MeshTools::create_local_bounding_box(_mesh.getMesh());

  // Gather the bounding boxes of all processors
  std::vector<std::pair<Point, Point>> bb_points = {static_cast<std::pair<Point, Point>>(bbox)};
  comm().allgather(bb_points, true);

  // Inflate the local bboxes by a bit and store
  _inflated_bboxes.resize(comm().size());
  for (processor_id_type pid = 0; pid < comm().size(); ++pid)
  {
    BoundingBox pid_bbox = static_cast<BoundingBox>(bb_points[pid]);
    pid_bbox.scale(0.01);
    _inflated_bboxes[pid] = pid_bbox;
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

void
ClaimRays::verifyClaiming()
{
  // NOTE for all of the following: we use char here in place of bool.
  // This is because bool is not instantiated as a StandardType in
  // TIMPI due to the fun of std::vector<bool>

  // Map from Ray ID -> whether or not it was generated (false) or
  // claimed/possibly also generated (true)
  std::map<RayID, char> local_map;
  auto add_to_local_map =
      [this, &local_map](const std::vector<std::shared_ptr<Ray>> & rays, const bool claimed_rays)
  {
    for (const auto & ray : rays)
    {
      const auto id = getID(ray);

      // Try to insert into the map
      auto emplace_pair = local_map.emplace(id, claimed_rays);

      // If it already exists but has not been claimed yet, set it to being claimed
      if (!emplace_pair.second && claimed_rays)
      {
        mooseAssert(!emplace_pair.first->second,
                    "Ray was claimed more than once on a single processor");
        emplace_pair.first->second = true;
      }
    }
  };

  // Build the local_map
  add_to_local_map(_rays, false);
  add_to_local_map(_local_rays, true);

  // Build the structure to send the local generation/claiming information to rank 0
  std::map<processor_id_type, std::vector<std::pair<RayID, char>>> send_info;
  if (local_map.size())
    send_info.emplace(std::piecewise_construct,
                      std::forward_as_tuple(0),
                      std::forward_as_tuple(local_map.begin(), local_map.end()));

  // The mapping (filled on rank 0) from Ray ID -> (processor id, claiming status)
  std::map<RayID, std::vector<std::pair<processor_id_type, char>>> global_map;

  // Functor for receiving the generation/claiming information
  auto receive_functor = [&global_map](processor_id_type pid,
                                       const std::vector<std::pair<RayID, char>> & id_claimed_pairs)
  {
    for (const auto & id_claimed_pair : id_claimed_pairs)
      global_map[id_claimed_pair.first].emplace_back(pid, id_claimed_pair.second);
  };

  // Send claiming information to rank 0
  Parallel::push_parallel_vector_data(comm(), send_info, receive_functor);

  // Rank 0 will make sure everything looks good
  if (_pid == 0)
    for (const auto & id_pairs_pair : global_map)
    {
      const RayID id = id_pairs_pair.first;
      const std::vector<std::pair<processor_id_type, char>> & pid_claimed_pairs =
          id_pairs_pair.second;

      std::vector<processor_id_type> claimed_pids;
      for (const auto & pid_claimed_pair : pid_claimed_pairs)
        if (pid_claimed_pair.second)
          claimed_pids.push_back(pid_claimed_pair.first);

      if (claimed_pids.size() == 0)
        _study.mooseError("Failed to claim the Ray with ID ", id);
      mooseAssert(claimed_pids.size() == 1, "Ray was claimed on multiple processors");
    }
}
