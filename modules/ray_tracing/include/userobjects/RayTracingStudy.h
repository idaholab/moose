//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

// Local Includes
#include "ElemIndexHelper.h"
#include "ParallelRayStudy.h"
#include "RayTracingAttributes.h"
#include "TraceData.h"
#include "TraceRayBndElement.h"

// MOOSE Includes
#include "TheWarehouse.h"

// libMesh includes
#include "libmesh/mesh.h"
#include "libmesh/elem_side_builder.h"

// Forward Declarations
class RayBoundaryConditionBase;
class RayKernelBase;
class TraceRay;
class RayTracingObject;

/**
 * Base class for Ray tracing studies that will generate Rays and then propagate
 * all of them to termination.
 *
 * Subclasses _must_ override generateRays()
 */
class RayTracingStudy : public GeneralUserObject
{
public:
  RayTracingStudy(const InputParameters & parameters);

  /**
   * Key that is used for restricting access to acquireRayInternal().
   *
   * In general, we restrict access to the construction of Rays to only protected
   * methods within RayTracingStudy. However, there are a few classes/functions
   * that have a legimate reason to construct Rays. Here, the friends are said
   * classes/functions.
   */
  class AcquireRayInternalKey
  {
    friend class Parallel::Packing<std::shared_ptr<Ray>>;
    friend void dataLoad(std::istream & stream, std::shared_ptr<Ray> & ray, void * context);
    AcquireRayInternalKey() {}
    AcquireRayInternalKey(const AcquireRayInternalKey &) {}
  };

  /**
   * Key that is used for restricting access to moveRayToBufferDuringTrace() and
   * acquireRayDuringTrace().
   */
  class AcquireMoveDuringTraceKey
  {
    friend class RayBoundaryConditionBase;
    friend class RayKernelBase;
    AcquireMoveDuringTraceKey() {}
    AcquireMoveDuringTraceKey(const AcquireMoveDuringTraceKey &) {}
  };

  static InputParameters validParams();

  virtual void initialSetup() override;
  virtual void residualSetup() override;
  virtual void jacobianSetup() override;
  virtual void meshChanged() override;
  virtual void timestepSetup() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

  /**
   * Executes the study (generates and propagates Rays)
   */
  virtual void execute() override;

  /**
   * Setup for on subdomain change or subdomain AND ray change during ray tracing
   * @param subdomain The subdomain changed to
   * @param tid Thread id
   * @param ray_id ID of the ray initiating the change
   */
  virtual void
  segmentSubdomainSetup(const SubdomainID subdomain, const THREAD_ID tid, const RayID ray_id);

  /**
   * Reinitialize objects for a Ray segment for ray tracing
   * @param elem The elem the segment is in
   * @param start Start point of the segment
   * @param end End point of the segment
   * @param length The length of the start -> end segment
   * @param tid Thread id
   */
  virtual void reinitSegment(
      const Elem * elem, const Point & start, const Point & end, const Real length, THREAD_ID tid);

  /**
   * Called at the end of a Ray segment
   * @param tid Thread id
   * @param ray The ray
   */
  virtual void postOnSegment(const THREAD_ID tid, const std::shared_ptr<Ray> & ray);

  /**
   * Called at the beginning of a trace for a ray
   * @param tid Thread id
   * @param ray The ray
   */
  virtual void preTrace(const THREAD_ID /* tid */, const std::shared_ptr<Ray> & /* ray */) {}

  /**
   * Method for executing the study so that it can be called out of the standard UO execute()
   */
  void executeStudy();

  /**
   * Total number of processor crossings for Rays that finished on this processor
   */
  unsigned long long int endingProcessorCrossings() const { return _ending_processor_crossings; }
  /**
   * Max number of total processor crossings for Rays that finished on this processor
   */
  unsigned int endingMaxProcessorCrossings() const { return _ending_max_processor_crossings; }
  /**
   * Total number of processor crossings
   */
  unsigned long long int totalProcessorCrossings() const { return _total_processor_crossings; }
  /**
   * Max number of processor crossings for all Rays
   */
  unsigned int maxProcessorCrossings() const { return _max_processor_crossings; }

  /**
   * Total number of Ray/element intersections for Rays that finished on this processor
   */
  unsigned long long int endingIntersections() const { return _ending_intersections; }
  /**
   * Max number of intersections for Rays that finished on this processor
   */
  unsigned int endingMaxIntersections() const { return _ending_max_intersections; }
  /**
   * Total number of Ray/element intersections
   */
  unsigned long long int totalIntersections() const { return _total_intersections; }
  /**
   * Max number of intersections for a Ray
   */
  unsigned int maxIntersections() const { return _max_intersections; }
  /**
   * Max number of trajectory changes for a Ray
   */
  unsigned int maxTrajectoryChanges() const { return _max_trajectory_changes; }

  /**
   * Total amount of distance traveled by the rays that end on this processor
   */
  Real endingDistance() const { return _ending_distance; }
  /**
   * Total distance traveled by all Rays
   */
  Real totalDistance() const { return _total_distance; }

  unsigned long long int localTraceRayResult(const int result) const
  {
    return _local_trace_ray_results[result];
  }

  const ParallelRayStudy & parallelRayStudy() const { return *_parallel_ray_study; }

  /**
   * Max distance any Ray can travel
   */
  Real rayMaxDistance() const { return _ray_max_distance; }

  /**
   * Duration for execute() in seconds
   */
  Real executionTime() { return std::chrono::duration<Real>(_execution_time).count(); }
  /**
   * Duration for execute() in nanoseconds
   */
  Real executionTimeNano()
  {
    return std::chrono::duration<Real, std::nano>(_execution_time).count();
  }
  /**
   * Duration for creation of all Rays in seconds
   */
  Real generationTime() const { return std::chrono::duration<Real>(_generation_time).count(); }
  /**
   * Duration for creation of all Rays in seconds
   */
  Real propagationTime() const { return std::chrono::duration<Real>(_propagation_time).count(); }

  /**
   * Whether or not to tolerate failure
   */
  bool tolerateFailure() const { return _tolerate_failure; }

  /**
   * Whether or not to bank Rays on completion
   */
  bool bankRaysOnCompletion() const { return _bank_rays_on_completion; }

  /**
   * Whether or not to use Ray dependent subdomain setup
   */
  bool rayDependentSubdomainSetup() const { return _ray_dependent_subdomain_setup; }

  /**
   * Register a value to be filled in the data on a Ray with a given name.
   * @param name The name to register
   * \return The Ray data index for the registered value
   *
   * Note that this does not actually allocate the storage. It is simply a registration system to
   * keep track of the values stored in the Ray data.
   */
  RayDataIndex registerRayData(const std::string & name);
  /**
   * Register values to be filled in the data on a Ray with a given name.
   * @param names The names to register
   * \return The Ray data indices for the registered value
   *
   * Note that this does not actually allocate the storage. It is simply a registration system to
   * keep track of the values stored in the Ray data.
   */
  std::vector<RayDataIndex> registerRayData(const std::vector<std::string> & names);
  /**
   * Gets the index associated with a registered value in the Ray data
   * @param name The value name to get the index of
   * @param graceful Whether or not to exit gracefully if none is found (return
   * INVALID_RAY_DATA_INDEX)
   * \return The index for the value
   */
  RayDataIndex getRayDataIndex(const std::string & name, const bool graceful = false) const;
  /**
   * Gets the indices associated with registered values in the Ray data
   * @param names The value names to get the indices of
   * @param graceful Whether or not to exit gracefully if none is found (index is filled with
   * INVALID_RAY_DATA_INDEX)
   * \return The indices for the values
   */
  std::vector<RayDataIndex> getRayDataIndices(const std::vector<std::string> & names,
                                              const bool graceful = false) const;
  /**
   * Gets the name associated with a registered value in the Ray data
   * @param index The index to get the name of
   * \return The name associated with index
   */
  const std::string & getRayDataName(const RayDataIndex index) const;
  /**
   * Gets the names associated with registered values in the Ray data
   * @param indices The indices to get the names of
   * \return The associated names
   */
  std::vector<std::string> getRayDataNames(const std::vector<RayDataIndex> & indices) const;

  /**
   * The registered size of values in the Ray data.
   */
  std::size_t rayDataSize() const { return _ray_data_names.size(); }
  /**
   * Whether or not any Ray data are registered
   */
  bool hasRayData() const { return _ray_data_names.size(); }
  /**
   * The Ray data names
   */
  const std::vector<std::string> & rayDataNames() const { return _ray_data_names; }

  /**
   * Register a value to be filled in the aux data on a Ray with a given name.
   * @param name The name to register
   * \returns The Ray aux data index for the registered value
   *
   * Note that this does not actually allocate the storage. It is simply a registration system to
   * keep track of the values stored in the Ray aux data.
   */
  RayDataIndex registerRayAuxData(const std::string & name);
  /**
   * Register values to be filled in the aux data on a Ray with a given name.
   * @param names The names to register
   * \returns The Ray aux data indices for the registered values
   *
   * Note that this does not actually allocate the storage. It is simply a registration system to
   * keep track of the values stored in the Ray aux data.
   */
  std::vector<RayDataIndex> registerRayAuxData(const std::vector<std::string> & names);
  /**
   * Gets the index associated with a registered value in the Ray aux data
   * @param name The value name to get the index of
   * @param graceful Whether or not to exit gracefully if none is found (return
   * INVALID_RAY_DATA_INDEX)
   * \returns The index for the value
   */
  RayDataIndex getRayAuxDataIndex(const std::string & name, const bool graceful = false) const;
  /**
   * Gets the indices associated with registered values in the Ray aux data
   * @param names The value names to get the indices of
   * @param graceful Whether or not to exit gracefully if none is found (index is filled with
   * INVALID_RAY_DATA_INDEX)
   * \return The indices for the values
   */
  std::vector<RayDataIndex> getRayAuxDataIndices(const std::vector<std::string> & names,
                                                 const bool graceful = false) const;
  /**
   * Gets the name associated with a registered value in the Ray aux data
   * @param index The index to get the name of
   * \return The name associated with index
   */
  const std::string & getRayAuxDataName(const RayDataIndex index) const;
  /**
   * Gets the names associated with registered values in the Ray aux data
   * @param indices The indices to get the names of
   * \return The associated names
   */
  std::vector<std::string> getRayAuxDataNames(const std::vector<RayDataIndex> & indices) const;
  /**
   * The registered size of values in the Ray aux data.
   */
  std::size_t rayAuxDataSize() const { return _ray_aux_data_names.size(); }
  /**
   * Whether or not any Ray aux data are registered
   */
  bool hasRayAuxData() const { return _ray_aux_data_names.size(); }
  /**
   * The Ray aux data names
   */
  const std::vector<std::string> & rayAuxDataNames() const { return _ray_aux_data_names; }

  /**
   * Whether or not there are currently any active RayKernel objects
   */
  bool hasRayKernels(const THREAD_ID tid);
  /**
   * Fills the active RayKernels associated with this study and a block into result
   */
  void getRayKernels(std::vector<RayKernelBase *> & result, SubdomainID id, THREAD_ID tid);
  /**
   * Fills the active RayKernels associated with this study into result
   */
  template <typename T>
  void getRayKernels(std::vector<T *> & result, THREAD_ID tid)
  {
    _fe_problem.theWarehouse()
        .query()
        .condition<AttribRayTracingStudy>(this)
        .condition<AttribSystem>("RayKernel")
        .condition<AttribThread>(tid)
        .queryInto(result);
  }
  /**
   * Fills the active RayKernels associeted with this study, block, and potentially Ray into result
   */
  void
  getRayKernels(std::vector<RayKernelBase *> & result, SubdomainID id, THREAD_ID tid, RayID ray_id);
  /**
   * Fills the active RayBCs associated with this study and a boundary into result
   */
  void getRayBCs(std::vector<RayBoundaryConditionBase *> & result, BoundaryID id, THREAD_ID tid);
  /**
   * Fills the active RayBCs associated with this study and boundaries result
   */
  template <typename T>
  void getRayBCs(std::vector<T *> & result, const std::vector<BoundaryID> & ids, THREAD_ID tid)
  {
    _fe_problem.theWarehouse()
        .query()
        .condition<AttribRayTracingStudy>(this)
        .condition<AttribSystem>("RayBoundaryCondition")
        .condition<AttribBoundaries>(ids)
        .condition<AttribThread>(tid)
        .queryInto(result);
  }
  /**
   * Fills the active RayBCs associated with this study into result
   */
  template <typename T>
  void getRayBCs(std::vector<T *> & result, THREAD_ID tid)
  {
    _fe_problem.theWarehouse()
        .query()
        .condition<AttribRayTracingStudy>(this)
        .condition<AttribSystem>("RayBoundaryCondition")
        .condition<AttribThread>(tid)
        .queryInto(result);
  }
  /**
   * Fills the active RayBCs associated with thie study, boundary elements, and potentially Ray into
   * result.
   *
   * This is purposely virtual because it allows derived studies to optimize the retrieval of RayBCs
   * during the trace in TraceRay.
   */
  virtual void getRayBCs(std::vector<RayBoundaryConditionBase *> & result,
                         const std::vector<TraceRayBndElement> & bnd_elems,
                         THREAD_ID tid,
                         RayID ray_id);

  /**
   * Gets the current RayKernels for a thread, which are set in segmentSubdomainSetup()
   */
  const std::vector<RayKernelBase *> & currentRayKernels(THREAD_ID tid) const
  {
    return _threaded_current_ray_kernels[tid];
  }

  /**
   * Get the nodal bounding box for the domain
   */
  const BoundingBox & boundingBox() const { return _b_box; }
  /**
   * Get the loose nodal bounding box for the domain
   *
   * BoundingBox::contains_point does not have a tolerance, so we need to stretch the
   * box a little bit for some contains_point checks.
   */
  const BoundingBox & looseBoundingBox() const { return _loose_b_box; }
  /**
   * Get the inflated maximum length across the domain
   */
  Real domainMaxLength() const { return _domain_max_length; }
  /**
   * Get the current total volume of the domain
   */
  Real totalVolume() const { return _total_volume; }
  /**
   * Whether or not the domain is rectangular (if it is prefectly encompassed by its bounding box)
   */
  bool isRectangularDomain() const;

  /**
   * Whether or not the local mesh has internal sidesets that have RayBCs on them
   *
   * NOTE: if useInternalSidesets() == false, this will be false even if the mesh does
   * have internal sidesets
   */
  bool hasInternalSidesets() const { return _internal_sidesets.size(); }
  /**
   * Get the internal sidesets (that have RayBC(s)) for each side for a given element
   *
   * This will be empty if the elem does not have any internal sidesets that have RayBC(s)
   */
  const std::vector<std::vector<BoundaryID>> & getInternalSidesets(const Elem * elem) const;
  /**
   * Gets the internal sidesets (that have RayBCs) within the local domain
   */
  const std::set<BoundaryID> & getInternalSidesets() const { return _internal_sidesets; }
  /**
   * Whether or not the side \s on elem \p elem is non-planar
   *
   * This is cached because checking whether or not a face is planar is costly
   */
  bool sideIsNonPlanar(const Elem * elem, const unsigned short s) const
  {
    return _has_non_planar_sides && _non_planar_sides[_elem_index_helper.getIndex(elem)][s] == 1;
  }
  /**
   * Whether or not the mesh has active elements of the same level
   *
   * Use this over sameLevelActiveElems(), which is for internally setting
   * _has_same_level_active_elems
   */
  bool hasSameLevelActiveElems() const { return _has_same_level_active_elems; }

  /**
   * INTERNAL methods for acquiring a Ray during a trace in RayKernels and RayBCs.
   *
   * You should not use these APIs directly. If you wish to acquire a Ray during generation
   * during generateRays()), use the protected RayTracingStudy::acquire{}Ray() methods.
   * If you wish to acquire a Ray during propagation in RayKernels and RayBC, use the
   * protected RayKernelBase::acquireRay() and RayBoundaryConditionBase::acquireRay(),
   * respectively.
   */
  ///@{
  std::shared_ptr<Ray> acquireRayDuringTrace(const THREAD_ID tid,
                                             const AcquireMoveDuringTraceKey &);
  std::shared_ptr<Ray> acquireRayInternal(const RayID id,
                                          const std::size_t data_size,
                                          const std::size_t aux_data_size,
                                          const bool reset,
                                          const AcquireRayInternalKey &)
  {
    return _parallel_ray_study->acquireParallelData(
        0, this, id, data_size, aux_data_size, reset, Ray::ConstructRayKey());
  }
  ///@}

  /**
   * INTERNAL method for moving a Ray into the buffer during tracing.
   *
   * You should not and cannot use this API. It is protected by the
   * AcquireMoveDuringTraceKey, which is only constructable by
   * RayKernelBase and RayBoundaryConditionBase.
   */
  void moveRayToBufferDuringTrace(std::shared_ptr<Ray> & ray,
                                  const THREAD_ID tid,
                                  const AcquireMoveDuringTraceKey &);
  /**
   * Access to the libMesh MeshBase.
   *
   * This is needed for unpack routines for a Ray, which has a context of this study.
   */
  MeshBase & meshBase() const { return _mesh; }

  /**
   * @returns A reference to the MooseMesh associated with this study.
   */
  MooseMesh & mesh() { return _mesh; }

  /**
   * Get the outward normal for a given element side.
   */
  virtual const Point &
  getSideNormal(const Elem * elem, const unsigned short side, const THREAD_ID tid);
  /**
   * Gets the outward normals for a given element.
   * Returns a pointer to the normal for the zeroth side.
   */
  virtual const Point * getElemNormals(const Elem * /* elem */, const THREAD_ID /* tid */)
  {
    mooseError("Unimplemented element normal caching in ", type(), "::getElemNormals()");
  }

  /**
   * Gets the data value for a banked ray with a given ID
   *
   * This will return the value replicated across all processors
   */
  RayData getBankedRayData(const RayID ray_id, const RayDataIndex index) const;
  /**
   * Gets the data value for a banked ray with a given ID
   *
   * This will return the value replicated across all processors
   */
  RayData getBankedRayAuxData(const RayID ray_id, const RayDataIndex index) const;

  /**
   * Gets the ID of a registered ray
   * @param name The name of said ray
   * @param graceful Whether or not to exit gracefully if none is found (with invalid_id)
   */
  RayID registeredRayID(const std::string & name, const bool graceful = false) const;
  /**
   * Gets the name of a registered ray
   * @param ray_id The ID of said ray
   */
  const std::string & registeredRayName(const RayID ray_id) const;

  /**
   * Whether or not ray registration is being used
   */
  bool useRayRegistration() const { return _use_ray_registration; }

  /**
   * Whether or not to store the Ray data on the cached Ray traces
   */
  bool dataOnCacheTraces() const { return _data_on_cache_traces; }
  /**
   * Whether or not to store the Ray aux data on the cached Ray traces
   */
  bool auxDataOnCacheTraces() const { return _aux_data_on_cache_traces; }
  /**
   *  Whether or not to cache individual element segments when _cache_traces = true
   */
  bool segmentsOnCacheTraces() const { return _segments_on_cache_traces; }

  /**
   * Virtual that allows for selection in if a Ray should be cached or not (only used when
   * _cache_traces).
   */
  virtual bool shouldCacheTrace(const std::shared_ptr<Ray> & /* ray */) const
  {
    return _always_cache_traces;
  }

  /**
   * Initialize a Ray in the threaded cached trace map to be filled with segments
   */
  TraceData & initThreadedCachedTrace(const std::shared_ptr<Ray> & ray, THREAD_ID tid);
  /**
   * Get the cached trace data structure
   */
  const std::vector<TraceData> & getCachedTraces() const { return _cached_traces; }

  /**
   * Get the cached hmax for all elements in a subdomain
   *
   * Used for scaling tolerances in ray tracing.
   */
  Real subdomainHmax(const SubdomainID subdomain_id) const;

  /**
   * Entry point for acting on a ray when it is completed (shouldContinue() == false)
   */
  virtual void onCompleteRay(const std::shared_ptr<Ray> & ray);

  /**
   * Verifies that the Rays in the given range have unique Ray IDs.
   *
   * @param begin The beginning of the range of Rays to check
   * @param end The end of the range of Rays to check
   * @param global If true, this will complete the verification across all processors
   * @param error_suffix Entry point for additional information in the error message
   */
  void verifyUniqueRayIDs(const std::vector<std::shared_ptr<Ray>>::const_iterator begin,
                          const std::vector<std::shared_ptr<Ray>>::const_iterator end,
                          const bool global,
                          const std::string & error_suffix) const;

  /**
   * Verifies that the Rays in the given range are unique. That is, that there are not multiple
   * shared_ptrs that point to the same Ray
   *
   * @param begin The beginning of the range of Rays to check
   * @param end The end of the range of Rays to check
   * @param error_suffix Entry point for additional information in the error message
   */
  void verifyUniqueRays(const std::vector<std::shared_ptr<Ray>>::const_iterator begin,
                        const std::vector<std::shared_ptr<Ray>>::const_iterator end,
                        const std::string & error_suffix);

  /**
   * Whether or not the study is propagating (tracing Rays)
   */
  bool currentlyPropagating() const { return _parallel_ray_study->currentlyExecuting(); }
  /**
   * Whether or not the study is generating
   */
  bool currentlyGenerating() const { return _parallel_ray_study->currentlyPreExecuting(); }

  /**
   * Gets the threaded TraceRay object for \p tid.
   */
  ///@{
  TraceRay & traceRay(const THREAD_ID tid) { return *_threaded_trace_ray[tid]; }
  const TraceRay & traceRay(const THREAD_ID tid) const { return *_threaded_trace_ray[tid]; }
  ///@}

  /**
   * Whether or not to verify if Rays have valid information before being traced
   */
  bool verifyRays() const { return _verify_rays; }
  /**
   * Whether or not trace verification is enabled in devel/dbg modes
   */
#ifndef NDEBUG
  bool verifyTraceIntersections() const { return _verify_trace_intersections; }
#endif

  /**
   * Whether or not \p side is incoming on element \p elem in direction \p direction.
   */
  bool sideIsIncoming(const Elem * const elem,
                      const unsigned short side,
                      const Point & direction,
                      const THREAD_ID tid);

  /**
   * Whether or not to produce a warning when interacting with a non-planar mesh.
   */
  bool warnNonPlanar() const { return _warn_non_planar; }

  /**
   * The underlying parallel study: used for the context for calling the packed range routines.
   */
  ParallelStudy<std::shared_ptr<Ray>, Ray> * parallelStudy() { return _parallel_ray_study.get(); }

  /**
   * Get an element's side pointer without excessive memory allocation
   *
   * @param elem The element to build a side for
   * @param s The side to build
   * @param tid The thread id
   * @return A pointer to the side element
   */
  const libMesh::Elem &
  elemSide(const libMesh::Elem & elem, const unsigned int s, const THREAD_ID tid = 0)
  {
    return _threaded_elem_side_builders[tid](elem, s);
  }

protected:
  /**
   * Subclasses should override this to determine how to generate Rays.
   * This will be called within execute() and makes up the "generation phase"
   * of the algorithm.
   */
  virtual void generateRays() = 0;

  /**
   * Entry point before study execution
   */
  virtual void preExecuteStudy() {}
  /**
   * Entry point after study execution
   */
  virtual void postExecuteStudy() {}

  /**
   * Helper function for computing the total domain volume
   */
  Real computeTotalVolume();

  /**
   * Gets the writeable current RayKernels for a thread
   *
   * Allows for other ray studies to fill the current ray kernels in a custom manner
   */
  std::vector<RayKernelBase *> & currentRayKernelsWrite(THREAD_ID tid)
  {
    return _threaded_current_ray_kernels[tid];
  }

  /**
   * Reserve \p size entires in the Ray buffer.
   *
   * This can only be used within generateRays() and should be used when possible
   * to reserve entires in the buffer before adding Rays via moveRay(s)ToBuffer.
   */
  void reserveRayBuffer(const std::size_t size);

  /**
   * Determine whether or not the mesh currently has active elements that are
   * all the same level
   */
  bool sameLevelActiveElems() const;

  /**
   * Builds quadrature points for a given segment using the _segment_qrule
   * @param start Start point of the segment
   * @param end End point of the segment
   * @param length The lengh of the start -> end segment
   * @param points Points to fill into (should be sized ahead of time)
   * @param weights Weights to fill into (should be sized ahead of time)
   */
  virtual void buildSegmentQuadrature(const Point & start,
                                      const Point & end,
                                      const Real length,
                                      std::vector<Point> & points,
                                      std::vector<Real> & weights) const;

  /**
   * Get the Ray bank. This is the bank of Rays that have completed on this processor
   * after an execution of the study.
   *
   * This is only available when the private parameter _bank_rays_on_completion
   * is set to true.
   */
  const std::vector<std::shared_ptr<Ray>> & rayBank() const;

  /**
   * Gets the Ray with the ID \p ray_id from the Ray bank.
   *
   * If the Ray with \p ray_id is not found across all processors, this will error.
   *
   * This will ONLY return a valid Ray (not a null shared_ptr) on the processor that
   * has the Ray.
   */
  std::shared_ptr<Ray> getBankedRay(const RayID ray_id) const;

  /**
   * Resets the generation of unique RayIDs via generateUniqueRayID() to the beginning
   * of the range.
   */
  void resetUniqueRayIDs();
  /**
   * Resets the generation of unique replicated RayIDs accessed via generateReplicatedRayID().
   */
  void resetReplicatedRayIDs();

  /**
   * Gets all of the currently active RayTracingObjects
   */
  std::vector<RayTracingObject *> getRayTracingObjects();

  /**
   * Generates a unique RayID to be used for a Ray.
   *
   * This is used internally when acquiring new Rays.
   */
  virtual RayID generateUniqueRayID(const THREAD_ID tid);
  /**
   * Generates a Ray ID that is replicated across all processors.
   */
  RayID generateReplicatedRayID();

  /**
   * User APIs for constructing Rays within the RayTracingStudy.
   *
   * Rays can ONLY be constructed by users within the RayTracingStudy via the following methods.
   */
  ///@{
  /**
   * Acquire a Ray from the pool of Rays within generateRays().
   *
   * A unique ID is generated and assigned to the acquired Ray. The data and aux
   * data sizes are set according to the sizes required by the RayTracingStudy.
   */
  std::shared_ptr<Ray> acquireRay();
  /**
   * Acquire a Ray from the pool of Rays within generateRays(), without resizing
   * the data (sizes the data to zero).
   *
   * A unique ID is generated and assigned to the acquired Ray.
   */
  std::shared_ptr<Ray> acquireUnsizedRay();
  /**
   * Acquire a Ray from the pool of Rays within generateRays() in a replicated fashion.
   *
   * That is, this method must be called on all processors at the same time and the ID of the
   * resulting Ray is the same across all processors.
   *
   * The data and aux data sizes are set according to the sizes required by the RayTracingStudy.
   */
  std::shared_ptr<Ray> acquireReplicatedRay();
  /**
   * Acquires a Ray that that is copied from another Ray within generateRays().
   *
   * All of the information is copied except for the counters (intersections, processor crossings,
   * etc), which are reset.
   */
  std::shared_ptr<Ray> acquireCopiedRay(const Ray & ray);
  /**
   * Acquires a Ray with a given name within generateRays(). Used when ray registration is enabled,
   * that is, the private paramater '_use_ray_registration' == true.
   *
   * This method must be called on all processors at the same time with the same name.
   * This method can only be called on thread 0, which is why there is no thread argument.
   */
  std::shared_ptr<Ray> acquireRegisteredRay(const std::string & name);
  ///@}

  /**
   * Moves a ray to the buffer to be traced during generateRays().
   *
   * This moves the Ray into the buffer (with std::move), therefore \p ray will be nullptr after
   * this call.
   */
  void moveRayToBuffer(std::shared_ptr<Ray> & ray);
  /**
   * Moves rays to the buffer to be traced during generateRays().
   *
   * This moves the rays into the buffer (with std::move), therefore all valid rays
   * in \p rays will be nullptr after this call.
   */
  void moveRaysToBuffer(std::vector<std::shared_ptr<Ray>> & rays);

  /// The Mesh
  MooseMesh & _mesh;
  /// The Communicator
  const Parallel::Communicator & _comm;
  /// The rank of this processor (this actually takes time to lookup - so just do it once)
  const processor_id_type _pid;

  /// Whether or not to perform coverage checks on RayKernels
  const bool _ray_kernel_coverage_check;
  /// Whether not to warn if non-planar faces are found
  const bool _warn_non_planar;
  /// Whether or not to use Ray registration
  const bool _use_ray_registration;
  /// Whether or not to use the internal sidesets in ray tracing
  const bool _use_internal_sidesets;
  /// Whether or not to tolerate a Ray Tracing failure
  const bool _tolerate_failure;
  /// Whether or not to bank rays on completion
  const bool _bank_rays_on_completion;
  /// Whether or not subdomain setup is dependent on the Ray
  const bool _ray_dependent_subdomain_setup;

  /// Whether or not to cache traces on every trace execution
  bool _always_cache_traces;
  /// Whether or not to store the Ray data on the cache traces
  const bool _data_on_cache_traces;
  /// Whether or not to store the Ray aux data on the cache traces
  const bool _aux_data_on_cache_traces;
  /// Whether or not to cache individual element segments when caching
  const bool _segments_on_cache_traces;
  /// Max distance a Ray can travel before being killed (can change)
  const Real _ray_max_distance;
  /// Whether or not to verify if Rays have valid information before being traced
  const bool _verify_rays;
  /// Whether or not to verify the trace intersections in devel and dbg modes
#ifndef NDEBUG
  const bool _verify_trace_intersections;
#endif

private:
  /**
   * Perform coverage checks (coverage of RayMaterials and RayKernels, if enabled)
   */
  void coverageChecks();

  /**
   * Perform checks to see if the listed dependencies in the RayTracingObjects exist
   */
  void dependencyChecks();
  /**
   * Verifies that the dependencies exist for a set of RayTracingObjects
   */
  void verifyDependenciesExist(const std::vector<RayTracingObject *> & rtos);

  /**
   * Check for if all of the element types in the mesh are supported by ray tracing
   */
  void traceableMeshChecks();

  /**
   * Does the setup for internal sidesets. This includes:
   * - Setting if the local mesh has internal sidesets that have RayBCs and on which
   *   boundary (stored in _internal_sidesets)
   * - Sets up the internal sideset mapping that maps element sides to the BoundaryIDs
   *   on said side that have internal sidesets that have RayBCs
   *   (stored in _internal_sidesets_map)
   *
   * If _use_internal_sidesets == false, this will still check to make sure that the
   * user does not have internal sidesets with RayBCs on them, and will report an error
   * if so.
   */
  void internalSidesetSetup();

  /**
   * Sets up the caching of whether or not each element side is non-planar, which is
   * stored in _non_planar_sides.
   *
   * This is done because this check is made within TraceRay, and determining whether
   * or not a side is non planar is quite expensive and we don't want to call it
   * mid-trace.
   */
  void nonPlanarSideSetup();

  /**
   * Sets up the _elem_index_helper, which is used for obtaining a contiguous index
   * for all elements that this processor knows about.
   *
   * Said index is used for quickly accessing things like internal sidesets and checking
   * if an element side is non-planar during tracing.
   */
  void localElemIndexSetup();

  /**
   * Sets up the maps from Ray to associated RayTracingObjects if _use_ray_registration.
   *
   * If ray registration is disabled, this makes sure no RayTracingObjects provided rays.
   */
  void registeredRaySetup();

  /**
   * Zero the AuxVariables that the registered AuxRayKernels contribute to.
   */
  void zeroAuxVariables();

  /**
   * Caches the hmax for all elements in each subdomain
   */
  void subdomainHMaxSetup();

  /**
   * Internal method for registering Ray data or Ray aux data with a name.
   */
  RayDataIndex registerRayDataInternal(const std::string & name, const bool aux);
  /**
   * Internal method for registering Ray data or Ray aux data with names.
   */
  std::vector<RayDataIndex> registerRayDataInternal(const std::vector<std::string> & names,
                                                    const bool aux);
  /**
   * Internal method for getting the index of Ray data or Ray aux data.
   */
  RayDataIndex
  getRayDataIndexInternal(const std::string & name, const bool aux, const bool graceful) const;
  /**
   * Internal method for getting the indicies of Ray data or Ray aux data.
   */
  std::vector<RayDataIndex> getRayDataIndicesInternal(const std::vector<std::string> & names,
                                                      const bool aux,
                                                      const bool graceful) const;

  /**
   * Internal method for getting the name of Ray data or Ray aux data.
   */
  const std::string & getRayDataNameInternal(const RayDataIndex index, const bool aux) const;

  /**
   * Internal method for getting the value (replicated across all processors) in a Ray's data
   * or aux data from the Ray banks.
   */
  RayData
  getBankedRayDataInternal(const RayID ray_id, const RayDataIndex index, const bool aux) const;

  /**
   * Registers a Ray with a given name.
   *
   * This is for internal use only and is an internal map for replicated Ray IDs -> names for
   * easy access for studies that have a small number of rays to generate.
   *
   * @param name The name to register
   * @return The allocated ID for the registered ray
   */
  RayID registerRay(const std::string & name);

  /// Threaded helpers for building element sides without extraneous allocation
  std::vector<libMesh::ElemSideBuilder> _threaded_elem_side_builders;

  /// Timing
  //@{
  std::chrono::steady_clock::time_point _execution_start_time;
  std::chrono::steady_clock::duration _execution_time;
  std::chrono::steady_clock::duration _generation_time;
  std::chrono::steady_clock::duration _propagation_time;
  //@}

  /// The map from Ray data names to index
  std::unordered_map<std::string, RayDataIndex> _ray_data_map;
  /// The map from Ray aux data names to index
  std::unordered_map<std::string, RayDataIndex> _ray_aux_data_map;

  /// The names for each Ray data entry
  std::vector<std::string> _ray_data_names;
  /// The names for each Ray aux data entry
  std::vector<std::string> _ray_aux_data_names;

  /// Map from registered Ray name to ID
  std::unordered_map<std::string, RayID> & _registered_ray_map;
  /// Map from registered Ray ID to name
  std::vector<std::string> & _reverse_registered_ray_map;

  /// Storage for the cached traces
  std::vector<TraceData> _cached_traces;
  /// The threaded storage for cached traces
  std::vector<std::vector<TraceData>> _threaded_cached_traces;

  /// Number of currently cached objects for Jacobian/residual for each thread
  std::vector<std::size_t> _num_cached;

  /// The BoundaryIDs on the local mesh that have internal RayBCs
  std::set<BoundaryID> _internal_sidesets;
  /// Internal sideset data, if internal sidesets exist (indexed with getLocalElemIndex())
  std::vector<std::vector<std::vector<BoundaryID>>> _internal_sidesets_map;
  /// Whether or not the local mesh has elements with non-planar sides
  bool _has_non_planar_sides;
  /// Non planar side data, which is for quick checking if an elem side is non-planar
  /// We use unsigned short here to avoid a std::vector<bool>; 0 = false, otherwise true
  std::vector<std::vector<unsigned short>> _non_planar_sides;
  /// Whether or not the mesh has active elements of the same level
  bool _has_same_level_active_elems;

  /// Nodal bounding box for the domain
  libMesh::BoundingBox _b_box;
  /// Loose nodal bounding box for the domain
  libMesh::BoundingBox _loose_b_box;
  /// An inflated max distance for the domain
  Real _domain_max_length;
  /// The total volume of the domain
  Real _total_volume;

  /// Threaded cached subdomain query for RayKernelBase objects pertaining to this study
  std::vector<TheWarehouse::QueryCache<AttribSubdomains>> _threaded_cache_ray_kernel;
  /// Threaded cached boundary query for RayBC objects pertaining to this study
  std::vector<TheWarehouse::QueryCache<AttribBoundaries>> _threaded_cache_ray_bc;
  /// Threaded storage for all of the RayTracingObjects associated with a single Ray
  std::vector<std::vector<std::set<const RayTracingObject *>>> _threaded_ray_object_registration;
  /// The current RayKernel objects for each thread
  std::vector<std::vector<RayKernelBase *>> _threaded_current_ray_kernels;
  /// The TraceRay objects for each thread (they do the physical tracing)
  std::vector<std::shared_ptr<TraceRay>> _threaded_trace_ray;
  /// Face FE used for computing face normals for each thread
  std::vector<std::unique_ptr<libMesh::FEBase>> _threaded_fe_face;
  /// Face quadrature used for computing face normals for each thread
  std::vector<std::unique_ptr<libMesh::QBase>> _threaded_q_face;
  /// Threaded cache for side normals that have been computed already during tracing
  std::vector<std::unordered_map<std::pair<const Elem *, unsigned short>, Point>>
      _threaded_cached_normals;
  /// Cumulative Ray bank - stored only when _bank_rays_on_completion
  std::vector<std::shared_ptr<Ray>> _ray_bank;
  /// Storage for the next available unique RayID, obtained via generateUniqueRayID()
  std::vector<RayID> _threaded_next_ray_id;
  /// Storage for the next available replicated RayID, obtained via generateReplicatedRayID()
  RayID _replicated_next_ray_id;

  /// The study that used is to actually execute (trace) the Rays
  const std::unique_ptr<ParallelRayStudy> _parallel_ray_study;

  /// Quadrature rule for laying points across a 1D ray segment
  std::unique_ptr<libMesh::QBase> _segment_qrule;

  /// Total number of processor crossings for Rays that finished on this processor
  unsigned long long int _ending_processor_crossings;
  /// Max number of total processor crossings for Rays that finished on this processor
  unsigned int _ending_max_processor_crossings;
  /// Total number of processor crossings
  unsigned long long int _total_processor_crossings;
  /// Max number of processor crossings for all Rays
  unsigned int _max_processor_crossings;

  /// Total number of Ray/element intersections for Rays that finished on this processor
  unsigned long long int _ending_intersections;
  /// Max number of intersections for Rays that finished on this processor
  unsigned int _ending_max_intersections;
  /// Max number of trajectory changes for Rays that finished on this processor
  unsigned int _ending_max_trajectory_changes;
  /// Total number of Ray/element intersections
  unsigned long long int _total_intersections;
  /// Max number of intersections for a single Ray
  unsigned int _max_intersections;
  /// Max number of trajectory changes for a single Ray
  unsigned int _max_trajectory_changes;

  /// Total distance traveled by Rays that end on this processor
  Real _ending_distance;
  /// Total distance traveled by all Rays
  Real _total_distance;

  /// Cumulative results on this processor from the threaded TraceRay objects
  std::vector<unsigned long long int> _local_trace_ray_results;

  /// The cached hmax for all elements in a subdomain
  std::unordered_map<SubdomainID, Real> _subdomain_hmax;

  /// Whether or not we've called initial setup - used to stop from late registration
  bool _called_initial_setup;

  /// Helper for defining a local contiguous index for each element
  ElemIndexHelper _elem_index_helper;

  /// Spin mutex object for locks
  mutable Threads::spin_mutex _spin_mutex;
};
