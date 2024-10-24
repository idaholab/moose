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
#include "DebugRay.h"
#include "ElemExtrema.h"
#include "NeighborInfo.h"
#include "RayTracingCommon.h"
#include "TraceRayBndElement.h"

// MOOSE Includes
#include "MooseMesh.h"
#include "MooseHashing.h"
#include "MooseTypes.h"
#include "StaticallyAllocatedSet.h"

// libMesh includes
#include "libmesh/enum_elem_type.h"
#include "libmesh/elem_side_builder.h"

// Forward declarations
class Ray;
class RayBoundaryConditionBase;
class RayKernelBase;
class RayTracingStudy;
class RayTracingObject;
struct TraceData;
namespace libMesh
{
class BoundaryInfo;
class Edge;
class Mesh;
}

/**
 * Traces Rays through the mesh on a single processor
 */
class TraceRay
{
public:
  TraceRay(RayTracingStudy & study, const THREAD_ID tid);

  virtual ~TraceRay() {}

  /**
   * Enum for the various results reported by this object
   */
  enum TraceRayResult
  {
    INTERSECTIONS = 0,              // intersections
    FACE_HITS = 1,                  // intersections at an element face
    VERTEX_HITS = 2,                // intersections at an element vertex
    EDGE_HITS = 3,                  // intersections at an element edge
    MOVED_THROUGH_NEIGHBORS = 4,    // intersections through point/vertex/edge neighbors
    BACKFACE_CULLING_SUCCESSES = 5, // face intersection calls that backface culling worked on
    BACKFACE_CULLING_FAILURES = 6,  // face interesction calls that backface culling failed on
    INTERSECTION_CALLS = 7,         // face intersection calls made
    VERTEX_NEIGHBOR_BUILDS = 8,     // builds for neighbors at a vertex
    VERTEX_NEIGHBOR_LOOKUPS = 9,    // lookups for neighbors at a vertex
    EDGE_NEIGHBOR_BUILDS = 10,      // builds for neighbors on an edge
    EDGE_NEIGHBOR_LOOKUPS = 11,     // lookups for neighborson an edge
    POINT_NEIGHBOR_BUILDS = 12,     // builds for point neighbors (non edge/vertex)
    FAILED_TRACES = 13,             // rays that fail (allowed when _tolerate_failure == true),
    ENDED_STATIONARY = 14           // rays the end because they are stationary
  };

  /**
   * Should be called immediately before calling any traces.
   */
  void preExecute();

  /**
   * Called on mesh change
   */
  void meshChanged();

  /**
   * Traces a ray.
   */
  void trace(const std::shared_ptr<Ray> & ray);

  /**
   * Get the various results reported by this object, indexed by TraceRayResult
   */
  std::vector<unsigned long long int> results() const { return _results; }

  /**
   * Enable or disable the use of element normals for backface culling through the getElemNormals()
   * method
   *
   * By default, the _study.getElemNormals() method is not implemented. This is because in order to
   * be able to obtain the element normals often enough during a trace, one needs to implement a
   * method to cache the normals. Otherwise, it is not worthwhile to use the normals during the
   * trace because the time spent computing the normals on the fly will be greater than the time
   * gain by using the normals in the tracing process.
   */
  void setBackfaceCulling(const bool backface_culling) { _backface_culling = backface_culling; }

  /**
   * Gets the current Ray that is being traced
   */
  const std::shared_ptr<Ray> * const & currentRay() const { return _current_ray; }
  /**
   * Gets the element that the current Ray is being traced in
   */
  const Elem * const & currentElem() const { return _current_elem; }
  /**
   * Gets the current intersection point for the Ray that is being traced.
   *
   * If the Ray ends within the current element, this will be the point at which
   * the Ray ends within the current element.
   */
  const Point & currentIntersectionPoint() const { return _intersection_point; }
  /**
   * Gets the current incoming point for the Ray that is being traced.
   *
   * This is ONLY guaranteed to be valid during execution of RayKernels!
   */
  const Point & currentIncomingPoint() const { return _incoming_point; }
  /**
   * Gets the current incoming side for the Ray that is being traced.
   *
   * This is only valid during onSegment() (when RayKernels are being called)
   */
  const unsigned short & currentIncomingSide() const { return _incoming_side; }
  /**
   * Gets the side that the current Ray intersected.
   *
   * If the ray ends within the current element, this will be
   * RayTracingCommon::invalid_side.
   */
  const unsigned short & currentIntersectedSide() const { return _intersected_side; }
  /**
   * Gets the element extrema (vertex/edge) that the current Ray intersected
   */
  const ElemExtrema & currentIntersectedExtrema() const { return _intersected_extrema; }
  /**
   * Gets the current intersection distance for the Ray that is being traced.
   *
   * When within an element (executing with a RayKernel), this is the segment distance.
   */
  const Real & currentIntersectionDistance() const { return _intersection_distance; }
  /**
   * Gets the BoundaryID of the boundary that the Ray intersected and is being applied
   * a boundary condition.
   */
  const BoundaryID & currentBoundaryID() const { return _current_bnd_id; }
  /**
   * Gets the subdomain of the current element that the Ray is being traced in.
   */
  const SubdomainID & currentSubdomainID() const { return _current_subdomain_id; }

  /**
   * Gets the neighbors at a vertex.
   * @param elem An elem that contains the vertex
   * @param vertex The Node that is the vertex
   */
  const std::vector<NeighborInfo> & getVertexNeighbors(const Elem * elem, const Node * vertex);
  /**
   * Gets the neighbors at a vertex.
   * @param elem An elem that contains the vertex
   * @param vertex The local ID of the vertex on elem
   */
  const std::vector<NeighborInfo> & getVertexNeighbors(const Elem * elem,
                                                       const unsigned short vertex);
  /**
   * Get the neighbors at an edge.
   * @param elem An elem that contains the edge
   * @param vertices A pair of Nodes that are the vertices that contain the edge fully
   * @param point The point on the edge where neighbors are desired
   */
  const std::vector<NeighborInfo> &
  getEdgeNeighbors(const Elem * elem,
                   const std::pair<const Node *, const Node *> & vertices,
                   const Point & point);
  /**
   * Get the neighbors at an edge.
   * @param elem An elem that contains the edge
   * @param vertices A pair of local vertex IDs of the vertices that contain the edge fully
   * @param point The point on the edge where neighbors are desired
   */
  const std::vector<NeighborInfo> &
  getEdgeNeighbors(const Elem * elem,
                   const std::pair<unsigned short, unsigned short> & vertices,
                   const Point & point);
  /**
   * Get the point neighbors
   * @param elem The elem
   * @param point The point
   */
  const std::vector<NeighborInfo> & getPointNeighbors(const Elem * elem, const Point & point);
  /**
   * Get the point/vertex/edge neighbors depending on extrema
   */
  const std::vector<NeighborInfo> &
  getNeighbors(const Elem * elem, const ElemExtrema & extrema, const Point & point);

private:
  /**
   * Called on a single segment traced by a Ray.
   */
  void onSegment(const std::shared_ptr<Ray> & ray);

  /**
   * Called when a Ray hits a boundary.
   */
  void onBoundary(const std::shared_ptr<Ray> & ray, const bool external);

  /**
   * Called when a Ray is finished tracing (whenever !ray->shouldContinue())
   * @param ray The ray that is finished tracing
   */
  void onCompleteTrace(const std::shared_ptr<Ray> & ray);
  /**
   * Called when a Ray is continuing to trace after segment
   * @param ray The ray
   */
  void onContinueTrace(const std::shared_ptr<Ray> & /* ray */);
  /**
   * Called when a Ray's trajectory changes
   * @param ray The ray
   */
  void onTrajectoryChanged(const std::shared_ptr<Ray> & ray);
  /**
   * Called when the subdomain changes.
   * @param ray The current Ray
   * @param same_ray Whether or not this is being called on the same Ray as previously
   */
  void onSubdomainChanged(const std::shared_ptr<Ray> & ray, const bool same_ray);

  /**
   * Creates a useful error string with current tracing information.
   *
   * If line is provided, will include the line number.
   */
  std::string failTraceMessage(const std::string & reason, const int line = -1);

  /**
   * Specialized mooseError for a failed Ray trace with detailed information regarding the trace.
   *
   * If warning = true, use mooseWarning instead.
   * If line is provided, output will include the line number.
   */
  void failTrace(const std::string & reason, const bool warning, const int line = -1);

  /**
   * Enum for the different exit results for exitElem()
   */
  enum ExitsElemResult
  {
    NO_EXIT = 0,    // doesn't exit
    HIT_FACE = 1,   // exits through a face
    HIT_VERTEX = 2, // exits through a vertex
    HIT_EDGE = 3    // exits through an edge
  };

  /**
   * Determines if _current_ray moving in direction _direction exits elem.
   * @param elem The element to check
   * @param elem_type The type of elem (used to avoid virtual function calls on elem)
   * @param incoming_side The incoming side (if any)
   * @param intersection_point To be modified with the intersection point, if any
   * @param intersected_side To be modified with the intersected side, if any
   * @param intersected_extrema To be modified with the intersected extremum
   * (vertex/edge), if any
   * @param intersection_distance To be modified with the intersection distance, if any
   * @param normals Pointer to the normals for the current element, if any
   * @return Whether or not _current_ray exits elem
   */
  ExitsElemResult exitsElem(const Elem * elem,
                            const ElemType elem_type,
                            const unsigned short incoming_side,
                            Point & intersection_point,
                            unsigned short & intersected_side,
                            ElemExtrema & intersected_extrema,
                            Real & intersection_distance,
                            const Point * normals);

  /**
   * Determines if _current_ray moving in direction _direction exits \p elem.
   *
   * This function is templated to work only with elements that are faces or cells (2D or 3D)
   *
   * @param elem The element to check
   * @param incoming_side The incoming side (if any)
   * @param intersection_point To be modified with the intersection point, if any
   * @param intersected_side To be modified with the intersected side, if any
   * @param intersected_extrema To be modified with the intersected extremum
   * (vertex/edge), if any
   * @param intersection_distance To be modified with the intersection distance, if any
   * @param normals Pointer to the normals for the current element, if any
   * @return Whether or not _current_ray exits elem through a face
   */
  template <typename T, typename FirstOrderT>
  typename std::enable_if<!std::is_base_of<Edge, T>::value, bool>::type
  exitsElem(const Elem * elem,
            const unsigned short incoming_side,
            Point & intersection_point,
            unsigned short & intersected_side,
            ElemExtrema & intersected_extrema,
            Real & intersection_distance,
            const Point * normals);

  /**
   * Determines if _current_ray moving in direction _direction exits \p elem.
   *
   * This function is templated to work only with elements that are edges (1D).
   *
   * @param elem The element to check
   * @param incoming_side The incoming side (if any)
   * @param intersection_point To be modified with the intersection point, if any
   * @param intersected_side To be modified with the intersected side, if any
   * @param intersected_extrema To be modified with the intersected extremum
   * (vertex/edge), if any
   * @param intersection_distance To be modified with the intersection distance, if any
   * @param normals Pointer to the normals for the current element, if any
   * @return Whether or not _current_ray exits elem through a face
   */
  template <typename T, typename FirstOrderT>
  typename std::enable_if<std::is_base_of<Edge, T>::value, bool>::type
  exitsElem(const Elem * elem,
            const unsigned short incoming_side,
            Point & intersection_point,
            unsigned short & intersected_side,
            ElemExtrema & intersected_extrema,
            Real & intersection_distance,
            const Point * normals);

  /**
   * Moves the Ray though neighbors (vertex/edge/point)
   * @param neighbors The neighbors to try to move through
   * @param last_elem The last element that was moved through - this one will be tried last
   * @param best_elem The resulting element for which an intersection was found
   * @param best_elem_incoming_side The incoming side on the resulting element for which
   * @return The trace result
   */
  ExitsElemResult moveThroughNeighbors(const std::vector<NeighborInfo> & neighbors,
                                       const Elem * last_elem,
                                       const Elem *& best_elem,
                                       unsigned short & best_elem_incoming_side);

  /**
   * Sees if a Ray can move through a neighbor (vertex/edge/point)
   * @param neighbor_info The NeighborInfo for the neighbor
   * @param incoming_side To be filled with the incoming side on the neighbor, if any
   * @param intersection_point To be filled with the intersection point on the neighbor, if any
   * @param intersected_side To be filled with the intersected side on the neighbor, if any
   * @param intersected_extrema To be filled with the intersected extrema on the neighbor, if any
   * @param intersection_distance To be filled with the intersection distance, if any
   * @return The trace result
   */
  ExitsElemResult moveThroughNeighbor(const NeighborInfo & neighbor_info,
                                      unsigned short & incoming_side,
                                      Point & intersection_point,
                                      unsigned short & intersected_side,
                                      ElemExtrema & intersected_extrema,
                                      Real & intersection_distance);

  /**
   * Gets and applies external boundary conditions in _current_elem on side _intersected_side at
   * _intersection_point.
   */
  void applyOnExternalBoundary(const std::shared_ptr<Ray> & ray);
  /**
   * Gets and applies internal boundary conditions (if any) from _current_elem, _last_elem, and
   * any other point neighbors that have internal sidesets at _intersection_point.
   */
  void applyOnInternalBoundary(const std::shared_ptr<Ray> & ray);
  /**
   * Helper for possibly storing boundary information in _boundary_elems, which is storage for
   * boundary elements (elem, side, bnd_id) that need to have RayBCs applied to them.
   *
   * For each BoundaryID in bnd_ids, a ConstBndElement will be added if said BoundaryID
   * does not already exist in _boundary_elems.
   *
   * @param elem The element to possibly add
   * @param side The side to possibly add
   * @param bnd_ids The BoundaryIDs to possibly add
   * @param extrema The intersected elem extrema (vertex/edge), if any
   */
  void possiblyAddToBoundaryElems(const Elem * elem,
                                  const unsigned short side,
                                  const std::vector<BoundaryID> & bnd_ids,
                                  const ElemExtrema & extrema);
  /**
   * Sets up a ray to continue tracing off processor
   * @param ray The ray
   * @param elem The element that is owned by another processor
   * @param incoming_side The incoming side elem
   * @param point The point on elem at which ray continues
   */
  void continueTraceOffProcessor(const std::shared_ptr<Ray> & ray);

  /**
   * Finds (if any) an element side that is on the boundary and is outgoing
   * at _intersection_point that is on the extrema _intersected_extrema for _current_elem.
   *
   * This is necessary when a Ray hits a point that is on the boundary but
   * is not actually on a boundary side
   *
   * Fills the found boundary side into \p boundary_side, the element said side is on into
   * \p boundary_elem and the extrema on said elem into \p boundary_extrema
   */
  void findExternalBoundarySide(unsigned short & boundary_side,
                                ElemExtrema & boundary_extrema,
                                const Elem *& boundary_elem);

  /**
   * Stores the result given by an intersection in _results as necessary
   */
  void storeExitsElemResult(const ExitsElemResult result);

  /**
   * Get the approximate subdomain hmax for an element.
   *
   * Uses the cached value of _current_subdomain_hmax if the element is in the cached subdomain.
   */
  Real subdomainHmax(const Elem * elem) const;

  /**
   * Called after executing a RayTracingObject (RayBCs and RayKernels)
   *
   * Verifies the configuration of ray->shouldContinue() and ray->trajectoryChanged()
   */
  void postRayTracingObject(const std::shared_ptr<Ray> & ray, const RayTracingObject * rto);

  /// The RayTracingStudy
  RayTracingStudy & _study;
  /// The MooseMesh
  MooseMesh & _mesh;
  /// The mesh dimension
  const unsigned int _dim;
  /// The BoundaryInfo for the mesh
  const BoundaryInfo & _boundary_info;
  /// The processor id
  const processor_id_type _pid;
  /// The thread id
  const THREAD_ID _tid;

  /// Whether or not to use element normals for backface culling
  bool _backface_culling;

  /// The TraceData for the current cached trace (if any)
  TraceData * _current_cached_trace;

  /// The current ray being traced
  const std::shared_ptr<Ray> * _current_ray;
  /// The element the current Ray is being traced in
  const Elem * _current_elem;
  /// The last element the current Ray was traced in
  const Elem * _last_elem;
  /// The current SubdomainID
  SubdomainID _current_subdomain_id;
  /// The current subdomain hmax
  Real _current_subdomain_hmax;
  /// The current elem type (constant on subdomain), used to avoid elem->type() calls
  libMesh::ElemType _current_elem_type;
  /// The number of sides on the current elem, used to avoid elem->n_sides() virtual calls
  unsigned short _current_elem_n_sides;
  /// The incoming point of the current Ray
  Point _incoming_point;
  /// The incoming side of the current Ray
  unsigned short _incoming_side;
  /// Whether or not the current Ray should continue
  bool _should_continue;

  /// The work point for the intersection of the current Ray
  Point _intersection_point;
  /// The work point for the intersected side of the current Ray
  unsigned short _intersected_side;
  /// The work point for the intersected vertex/edge vertices of the current Ray, if any
  ElemExtrema _intersected_extrema;
  /// The intersected vertex/edge vertices for the previous intersection, if any
  ElemExtrema _last_intersected_extrema;
  /// The work point for the intersection distance of the current Ray
  Real _intersection_distance;
  /// The current BoundaryID (used when calling RayBoundaryConditionBase::onBoundary())
  BoundaryID _current_bnd_id;

  /// Whether or not the current trace exits an element
  bool _exits_elem;

  /// The normals for the current element for backface culling (pointer to the first normal - optional)
  const Point * _current_normals;

  /// Reusable vector for calling _boundary_info.boundary_ids()
  std::vector<BoundaryID> _boundary_ids;
  /// Boundary elements that need RayBCs to be applied
  std::vector<TraceRayBndElement> _boundary_elems;

  /// The cached vertex neighbors
  std::unordered_map<const Node *, std::vector<NeighborInfo>> _vertex_neighbors;
  /// The cached edge neighbors
  std::unordered_map<std::pair<const Node *, const Node *>,
                     std::pair<bool, std::vector<NeighborInfo>>>
      _edge_neighbors;

  /// Reusable for building neighbors
  std::vector<NeighborInfo> _point_neighbor_helper;
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> _neighbor_set;
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> _neighbor_untested_set;
  MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> _neighbor_next_untested_set;
  std::vector<const Elem *> _neighbor_active_neighbor_children;

  /// Results over all of the local traces, indexed by TraceRayResult
  std::vector<unsigned long long int> _results;

  /// Helper for building element sides without excessive allocation
  libMesh::ElemSideBuilder _elem_side_builder;

  /// Reusable for getting the RayBCs in onBoundary()
  std::vector<RayBoundaryConditionBase *> _on_boundary_ray_bcs;
  /// Reusable for which boundary elements to apply for a specific RayBC in onBoundary()
  std::vector<std::size_t> _on_boundary_apply_index;

  /// Whether or not the RayTracingStudy has any RayKernels
  bool _has_ray_kernels;
  /// Whether or not the domain is rectangular (defined perfectly by its bounding box)
  bool _is_rectangular_domain;

  /// Helper for avoiding calling preTrace() on the same RayKernel multiple times
  std::set<RayKernelBase *> _old_ray_kernels;

#ifdef DEBUG_RAY_MESH_IF
  Mesh * _debug_mesh;
  Parallel::Communicator _debug_comm;
  unsigned int _debug_node_count;
#endif
};

// Helper for a mooseAssert() with useful trace information
#ifdef NDEBUG
#define traceAssert(asserted, msg) ((void)0)
#else
#define traceAssert(asserted, msg)                                                                 \
  do                                                                                               \
  {                                                                                                \
    if (!(asserted))                                                                               \
      mooseAssert(asserted, failTraceMessage(msg, __LINE__));                                      \
  } while (0)
#endif
