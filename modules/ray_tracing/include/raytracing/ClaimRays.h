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

// Forward declarations
class RayTracingStudy;
class MooseMesh;

/**
 * Helper object for claiming Rays
 */
class ClaimRays : public libMesh::ParallelObject, public MeshChangedInterface
{
public:
  /**
   * Constructor.
   * @param study The RayTracingStudy
   * @param parallel_study The base parallel study
   * @param mesh The MooseMesh
   * @param rays The vector of Rays that need to be claimed
   * @param local_rays Insertion point for Rays that have been claimed
   * @param do_exchange Whether or not an exchange is needed, i.e., if "rays" still needs to be
   * filled by objects on other processors
   */
  ClaimRays(RayTracingStudy & study,
            const std::vector<std::shared_ptr<Ray>> & rays,
            std::vector<std::shared_ptr<Ray>> & local_rays,
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
  virtual void prePossiblyClaimRay(const std::shared_ptr<Ray> & /* ray */) {}
  /**
   * Entry point for acting on a Ray after it is claimed
   */
  virtual void postClaimRay(std::shared_ptr<Ray> & ray, const Elem * elem);

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
  virtual RayID getID(const std::shared_ptr<Ray> & ray) const { return ray->id(); }

  /**
   * Get the inflated bounding box for rank \pid.
   */
  const libMesh::BoundingBox & inflatedBoundingBox(const processor_id_type pid) const
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
  ParallelStudy<std::shared_ptr<Ray>, Ray> & _parallel_study;

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
  void possiblyClaim(const std::shared_ptr<Ray> & obj);

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
  const std::vector<std::shared_ptr<Ray>> & _rays;
  /// The local Rays that are claimed
  std::vector<std::shared_ptr<Ray>> & _local_rays;

  /// The point locator
  std::unique_ptr<libMesh::PointLocatorBase> _point_locator = nullptr;

  /// The inflated bounding boxes for all processors
  std::vector<libMesh::BoundingBox> _inflated_bboxes;

  /// Map of point neighbors for each element
  std::unordered_map<dof_id_type, std::vector<const Elem *>> _elem_point_neighbors;

  /// Whether or not an init is needed (bounding boxes, neighbors)
  bool _needs_init;
};
