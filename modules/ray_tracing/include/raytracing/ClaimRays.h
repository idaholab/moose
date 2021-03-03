//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local includes
#include "ParallelStudy.h"
#include "Ray.h"

// MOOSE includes
#include "MeshChangedInterface.h"

// System includes
#include <unordered_map>

// libMesh includes
#include "libmesh/bounding_box.h"
#include "libmesh/parallel_object.h"
#include "libmesh/point_locator_base.h"
#include "timpi/request.h"

// Forward declarations
class RayTracingStudy;
class MooseMesh;

/**
 * Helper object for claiming Rays
 */
class ClaimRays : public ParallelObject, public MeshChangedInterface
{
public:
  /**
   * Constructor.
   * @param study The RayTracingStudy
   * @param parallel_study The base parallel study
   * @param mesh The MooseMesh
   * @param rays The vector of Rays that need to be claimed, to also be filled with the
   * claimed rays
   * @param do_exchange Whether or not an exchange is needed, i.e., if "rays" still needs to be
   * filled by objects on other processors
   */
  ClaimRays(RayTracingStudy & study,
            ParallelStudy<MooseUtils::SharedPool<Ray>::PtrType, Ray> & parallel_study,
            MooseMesh & mesh,
            std::vector<MooseUtils::SharedPool<Ray>::PtrType> & rays,
            const bool do_exchange);

  /**
   * Call on mesh changes to reinit the necessary data structures
   */
  virtual void meshChanged() override;

  /**
   * Claim the Rays
   *
   * \p do_exchange sets whether or not an exchange is needed, i.e., if _rays still needs
   * to be filled by objects on other processors
   */
  void claim();

protected:
  /**
   * Initialize the object
   */
  virtual void init();

  /**
   * Entry point before claim()
   */
  virtual void preClaim() {}
  /**
   * Entry point after claim()
   */
  virtual void postClaim() {}
  /**
   * Entry point before possibly claiming a Ray
   */
  virtual void prePossiblyClaimRay(Ray & /* ray */) {}
  /**
   * Entry point for acting on a Ray after it is claimed
   */
  virtual void postClaimRay(Ray & ray, const Elem * elem);

  /**
   * Gets an ID associated with the Ray for claiming purposes. Defaults
   * to the Ray's ID.
   *
   * To break ties in claiming (when multiple processors have elements that
   * contain a point, say on an element's side on a processor boundary),
   * we pick the smallest element ID when this ID is even and the
   * largest element ID is odd. It is possible that the same Rays can
   * be generated with different IDs, in which case the user may
   * want to use a different ID for this process.
   */
  virtual RayID getID(const Ray & ray) const { return ray.id(); }

  /**
   * Get the inflated bounding box for rank \pid.
   */
  const BoundingBox & inflatedBoundingBox(const processor_id_type pid) const
  {
    return _inflated_bboxes[pid];
  }

  /// The mesh
  MooseMesh & _mesh;
  /// This processor ID
  const processor_id_type _pid;

  /// Whether or not the Rays need to be initially exchanged
  const bool _do_exchange;

  /// The RayTracingStudy
  RayTracingStudy & _study;
  /// The ParallelStudy, used as the context for communicating rays
  ParallelStudy<MooseUtils::SharedPool<Ray>::PtrType, Ray> & _parallel_study;

private:
  /**
   * Builds the bounding boxes (_inflated_bboxes).
   */
  void buildBoundingBoxes();

  /**
   * Build the map of elements to all of their point neighbors
   *
   * TODO: Move this eventually into MooseMesh, MeshBase, or FEProblemBase
   */
  void buildPointNeighbors();

  /**
   * Possibly claim a Ray.
   */
  void possiblyClaim(MooseUtils::SharedPool<Ray>::PtrType && ray);

  /**
   * Verifies that the claiming process succeeded. That is, all Rays were claimed
   * once and only once.
   */
  void verifyClaiming();

  /**
   * Try to claim a spatial point.
   *
   * @param point The point to claim
   * @param id An ID associated with the point
   * @param elem The local element to first consider for this processor's ownership
   * @return The element that contains the point if we claim the point, nullptr if we don't claim it
   */
  const Elem * claimPoint(const Point & point, const RayID id, const Elem * elem);

  /// The Rays that need to be searched to possibly claimed
  std::vector<MooseUtils::SharedPool<Ray>::PtrType> & _rays;
  /// Temprorary for moving rays into before claiming
  std::vector<MooseUtils::SharedPool<Ray>::PtrType> _temp_rays;

  /// The point locator
  std::unique_ptr<PointLocatorBase> _point_locator;

  /// The inflated bounding boxes for all processors
  std::vector<BoundingBox> _inflated_bboxes;

  /// Map of point neighbors for each element
  std::unordered_map<dof_id_type, std::vector<const Elem *>> _elem_point_neighbors;

  /// Whether or not an init is needed (bounding boxes, neighbors)
  bool _needs_init;
};

template <typename MapToContainers,
          typename SendFunctor,
          typename PossiblyReceiveFunctor,
          typename ActionFunctor>
void
push_parallel_helper(const TIMPI::Communicator & comm,
                     MapToContainers & data,
                     SendFunctor & send_functor,
                     PossiblyReceiveFunctor & possibly_receive_functor,
                     ActionFunctor & act_on_data)
{
  typedef typename MapToContainers::value_type::second_type container_type;

  // This function must be run on all processors at once
  timpi_parallel_only(comm);

  // We'll grab a tag so we can overlap request sends and receives
  // without confusing one for the other
  auto tag = comm.get_unique_tag();

  // Save off the old send_mode so we can restore it after this
  auto old_send_mode = comm.send_mode();

  // Set the sending to synchronous - this is so that we can know when
  // the sends are complete
  const_cast<TIMPI::Communicator &>(comm).send_mode(TIMPI::Communicator::SYNCHRONOUS);

  // The send requests
  std::list<TIMPI::Request> requests;

  for (auto & datapair : data)
  {
    // In the case of data partitioned into more processors than we
    // have ranks, we "wrap around"
    processor_id_type dest_pid = datapair.first % comm.size();
    auto & datum = datapair.second;

    // Just act on data if the user requested a send-to-self
    if (dest_pid == comm.rank())
      act_on_data(dest_pid, datum);
    else
    {
      requests.emplace_back();
      send_functor(comm, dest_pid, datum, requests.back(), tag);
    }
  }

  // In serial we've now acted on all our data.
  if (comm.size() == 1)
    return;

  // Whether or not all of the sends are complete
  bool sends_complete = requests.empty();

  // Whether or not the nonblocking barrier has started
  bool started_barrier = false;
  // Request for the nonblocking barrier
  TIMPI::Request barrier_request;

  // The pair of src_pid and requests
  std::list<std::pair<unsigned int, std::shared_ptr<TIMPI::Request>>> receive_requests;
  auto current_request = std::make_shared<TIMPI::Request>();

  // Storage for the incoming data
  std::multimap<processor_id_type, std::shared_ptr<container_type>> incoming_data;
  auto current_incoming_data = std::make_shared<container_type>();

  unsigned int current_src_proc = 0;

  // Keep looking for receives
  while (true)
  {
    // Look for data from anywhere
    current_src_proc = TIMPI::any_source;

    // Check if there is a message and start receiving it
    if (possibly_receive_functor(
            comm, current_src_proc, *current_incoming_data, *current_request, tag))
    {
      receive_requests.emplace_back(current_src_proc, current_request);
      current_request = std::make_shared<TIMPI::Request>();

      // current_src_proc will now hold the src pid for this receive
      incoming_data.emplace(current_src_proc, current_incoming_data);
      current_incoming_data = std::make_shared<container_type>();
    }

    // Clean up outstanding receive requests
    receive_requests.remove_if(
        [&act_on_data,
         &incoming_data](std::pair<unsigned int, std::shared_ptr<TIMPI::Request>> & pid_req_pair) {
          auto & pid = pid_req_pair.first;
          auto & req = pid_req_pair.second;

          // If it's finished - let's act on it
          if (req->test())
          {
            // Do any post-wait work
            req->wait();

            auto it = incoming_data.find(pid);
            timpi_assert(it != incoming_data.end());

            act_on_data(pid, *it->second);

            // Don't need this data anymore
            incoming_data.erase(it);

            // This removes it from the list
            return true;
          }

          // Not finished yet
          return false;
        });

    requests.remove_if([](TIMPI::Request & req) {
      if (req.test())
      {
        // Do Post-Wait work
        req.wait();

        return true;
      }

      // Not finished yet
      return false;
    });

    // See if all of the sends are finished
    if (requests.empty())
      sends_complete = true;

    // If they've all completed then we can start the barrier
    if (sends_complete && !started_barrier)
    {
      started_barrier = true;
      comm.nonblocking_barrier(barrier_request);
    }

    // Must fully receive everything before being allowed to move on!
    if (receive_requests.empty())
      // See if all proessors have finished all sends (i.e. _done_!)
      if (started_barrier)
        if (barrier_request.test())
          break; // Done!
  }

  // Reset the send mode
  const_cast<TIMPI::Communicator &>(comm).send_mode(old_send_mode);
}

template <typename MapToContainers, typename ActionFunctor, typename Context>
void
new_push_parallel_packed_range(const TIMPI::Communicator & comm,
                               const MapToContainers & data,
                               Context * context,
                               const ActionFunctor & act_on_data)
{
  typedef typename MapToContainers::value_type::second_type container_type;
  typedef typename container_type::value_type nonref_type;
  typename std::remove_const<nonref_type>::type * output_type = nullptr;

  auto send_functor = [&context](const TIMPI::Communicator & comm,
                                 const processor_id_type dest_pid,
                                 container_type & datum,
                                 TIMPI::Request & request,
                                 const TIMPI::MessageTag tag) {
    comm.nonblocking_send_packed_range(dest_pid, context, datum.begin(), datum.end(), request, tag);
  };

  auto possibly_receive_functor = [&context, &output_type](const TIMPI::Communicator & comm,
                                                           unsigned int & current_src_proc,
                                                           container_type & current_incoming_data,
                                                           TIMPI::Request & current_request,
                                                           const TIMPI::MessageTag tag) {
    return comm.possibly_receive_packed_range(
        current_src_proc,
        context,
        std::inserter(current_incoming_data, current_incoming_data.end()),
        output_type,
        current_request,
        tag);
  };

  push_parallel_helper(comm,
                       const_cast<MapToContainers &>(data),
                       send_functor,
                       possibly_receive_functor,
                       act_on_data);
}

template <typename MapToVectors, typename ActionFunctor, typename Context>
void
new_push_parallel_vector_data(const TIMPI::Communicator & comm,
                              const MapToVectors & data,
                              const ActionFunctor & act_on_data)
{
  typedef typename MapToVectors::value_type::second_type container_type;

  typedef decltype(data.begin()->second.front()) ref_type;
  typedef typename std::remove_reference<ref_type>::type nonref_type;
  typedef typename std::remove_const<nonref_type>::type nonconst_nonref_type;

  // We'll construct the StandardType once rather than inside a loop.
  // We can't pass in example data here, because we might have
  // data.empty() on some ranks, so we'll need StandardType to be able
  // to construct the user's data type without an example.
  auto type = build_standard_type(static_cast<nonconst_nonref_type *>(nullptr));

  auto send_functor = [&type](const TIMPI::Communicator & comm,
                              const processor_id_type dest_pid,
                              container_type & datum,
                              TIMPI::Request & request,
                              const TIMPI::MessageTag tag) {
    comm.send(dest_pid, datum, type, request, tag);
  };

  auto possibly_receive_functor = [&type](const TIMPI::Communicator & comm,
                                          unsigned int & current_src_proc,
                                          container_type & current_incoming_data,
                                          TIMPI::Request & current_request,
                                          const TIMPI::MessageTag tag) {
    return comm.possibly_receive(
        current_src_proc, current_incoming_data, type, current_request, tag);
  };

  push_parallel_helper(
      comm, const_cast<MapToVectors &>(data), send_functor, possibly_receive_functor, act_on_data);
}
