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
#include "RayTracingCommon.h"

// MOOSE includes
#include "MooseError.h"
#include "MooseTypes.h"

// libMesh Includes
#include "libmesh/parallel.h"

// Forward declarations
namespace libMesh
{
class Elem;
}
class RayTracingStudy;
// Friend access to Ray
class TraceRay;
class TestRayLots;
// Friend access to ChangeDirectionKey for accessing changeDirection()
class RayBoundaryConditionBase;
// Friend access to ChangeStartDirectionKey for accessing changeStartDirection()
class RayKernelBase;
// Friend access to NonResetCountersKey for accessing constructor/reset without counter reset
namespace MooseUtils
{
template <class T>
class SharedPool;
}

/// Type for a Ray's ID
typedef unsigned long int RayID;
/// Type for a Ray's data
#ifdef SINGLE_PRECISION_RAY
typedef float RayData;
#else
typedef libMesh::Real RayData;
#endif
/// Type for the index into the data and aux data on a Ray
typedef unsigned int RayDataIndex;

/**
 * Basic datastructure for a ray that will traverse the mesh.
 */
class Ray
{
public:
  /**
   * Class that is used as a parameter to changeDirection() that allows only
   * RayBC methods to call changeDirection()
   */
  class ChangeDirectionKey
  {
    friend class RayBoundaryConditionBase;
    ChangeDirectionKey() {}
    ChangeDirectionKey(const ChangeDirectionKey &) {}
  };

  /**
   * Class that is used as a parameter to changeStartDirection() that allows only
   * RayKernelBase methods to call changeStartDirection()
   */
  class ChangeStartDirectionKey
  {
    friend class RayKernelBase;
    ChangeStartDirectionKey() {}
    ChangeStartDirectionKey(const ChangeStartDirectionKey &) {}
  };

  /**
   * Class that is used as a parameter to the public constructors/reset methods.
   *
   * We explicitly do not allow construction of Rays except through the
   * acquire{}() methods in the RayTracingStudy, which necessitates this class.
   */
  class ConstructRayKey
  {
    friend class RayTracingStudy;
    friend class MooseUtils::SharedPool<Ray>;
    friend class TestRayLots;
    ConstructRayKey() {}
    ConstructRayKey(const ConstructRayKey &) {}
  };

  /**
   * Ray constructor for internal use only.
   *
   * Even though this method is public, it in general CANNOT be used publicly. This is
   * ONLY used internally and is protected by the ConstructRayKey. In order to construct
   * a Ray as a user, use the RayTracingStudy::acquire{}() methods, such as
   * RayTracingStudy::acquireRay().
   *
   * @param study The study that owns the Ray
   * @param id ID for the Ray
   * @param data_size Size of data to initialize with zeros
   * @param aux_data_size Size of aux data to initialize with zeros
   * @param reset Whether or not to reset the Ray information
   */
  Ray(RayTracingStudy * study,
      const RayID id,
      const std::size_t data_size,
      const std::size_t aux_data_size,
      const bool reset,
      const ConstructRayKey &);

  /**
   * Resets a Ray for internal use only. Used by the SharedPool to reset a Ray with
   * these forwarded arguments.
   *
   * Even though this method is public, it in general CANNOT be used publicly. This is
   * ONLY used internally and is protected by the ConstructRayKey. In order to construct
   * a Ray as a user, use the RayTracingStudy::acquire{}() methods, such as
   * RayTracingStudy::acquireRay().
   *
   * @param study The study that owns the Ray
   * @param id ID for the Ray
   * @param data_size Size of data to initialize with zeros
   * @param aux_data_size Size of aux data to initialize with zeros
   * @param reset Whether or not to reset the Ray information
   */
  void reset(RayTracingStudy * study,
             const RayID id,
             const std::size_t data_size,
             const std::size_t aux_data_size,
             const bool reset,
             const ConstructRayKey &);

  /**
   * Copy constructor for internal use only.
   *
   * Even though this method is public, it in general CANNOT be used publicly. This is
   * ONLY used internally and is protected by the ConstructRayKey. In order to construct
   * a Ray as a user, use the RayTracingStudy::acquire{}() methods, such as
   * RayTracingStudy::acquireRay().
   *
   * Resets the counters.
   */
  Ray(const Ray * const other, const ConstructRayKey &);

  /**
   * Resets a Ray from another Ray for internal use only. Used by the SharedPool
   * to reset a Ray from another.
   *
   * Even though this method is public, it in general CANNOT be used publicly. This is
   * ONLY used internally and is protected by the ConstructRayKey. In order to construct
   * a Ray as a user, use the RayTracingStudy::acquire{}() methods, such as
   * RayTracingStudy::acquireRay().
   *
   * Resets the counters.
   */
  void reset(const Ray * const other, const ConstructRayKey &);

  /**
   * Deleted copy operator.
   *
   * General Ray modification is handled internally within the RayTracingStudy
   * via the RayTracingStudy::acquire{}() methods.
   */
  Ray & operator=(const Ray &) = delete;
  /**
   * Deleted default constructor.
   *
   * All Ray construction is handled internally within the RayTracingStudy
   * via the RayTracingStudy::acquire{}() methods.
   */
  Ray() = delete;
  /**
   * Deleted copy constructor.
   *
   * All Ray construction is handled internally within the RayTracingStudy
   * via the RayTracingStudy::acquire{}() methods.
   */
  Ray(const Ray & other) = delete;

  /**
   * Equality operator.
   *
   * This will perform "fuzzy" equals checks on the points and data.
   */
  bool operator==(const Ray & other) const { return equalityHelper(other, true); }
  /**
   * Non-equal operator.
   *
   * This will perform "fuzzy" equals checks on the points and data.
   */
  bool operator!=(const Ray & other) const { return equalityHelper(other, false); }

  /// Invalid index into a Ray's data
  static const RayDataIndex INVALID_RAY_DATA_INDEX = static_cast<RayDataIndex>(-1);
  /// Invalid Ray ID
  static const RayID INVALID_RAY_ID = static_cast<RayID>(-1);

  /**
   * Gets the Ray's ID
   */
  RayID id() const { return _id; }
  /**
   * Whether or not the Ray's ID is invalid
   */
  bool invalidID() const { return _id == INVALID_RAY_ID; }

  /**
   * Gets the point that the Ray is currently at.
   *
   * Before a Ray is traced, this is the starting point of the Ray.
   * While a Ray is being traced, this is the furthest point that the Ray
   * has travelled. During RayKernel execution, this is the end of the segment.
   * After a Ray has been traced, this is the point where the Ray was killed.
   */
  const Point & currentPoint() const { return _current_point; }
  /**
   * Whether or not the point that the Ray is currently at is valid.
   */
  bool invalidCurrentPoint() const { return _current_point == RayTracingCommon::invalid_point; }

  /**
   * This method is for internal use only. It is intended to be called only by
   * RayBoundaryConditionBase::changeRayDirection().
   *
   * If you wish to change a Ray's direction mid-trace in a RayBC, see
   * RayBoundaryConditionBase::changeRayDirection() instead.
   *
   * ChangeDirectionKey is constructable only by RayBC objects on purpose to limit usage of this
   * method.
   */
  void changeDirection(const Point & direction, const ChangeDirectionKey);

  /**
   * This method is for internal use only. It is intended to be called only by
   * RayKernelBase::changeRay().
   *
   * If you wish to change a Ray's direction mid-trace in a RayKernel, see
   * RayKernelBase::changeRay() instead.
   *
   * ChangeStartDirectionKey is constructable only by RayKernelBase objects on purpose to limit
   * usage of this method.
   */
  void
  changeStartDirection(const Point & start, const Point & direction, const ChangeStartDirectionKey);

  /**
   * Gets the Ray's direction
   */
  const Point & direction() const { return _direction; }
  /**
   * Whether or not the Ray's direction is set to invalid.
   */
  bool invalidDirection() const { return _direction == RayTracingCommon::invalid_point; }

  /**
   * Gets a writeable reference to the Ray's data.
   *
   * If the data is not sized to the size as required by the study, this will
   * resize the data.
   */
  std::vector<RayData> & data();
  /**
   * Gets a read only reference to the Ray's data.
   *
   * If the data is not sized to the size as required by the study, this will
   * resize the data.
   */
  const std::vector<RayData> & data() const;
  /**
   * Gets a writeable reference to the Ray's data at an index.
   *
   * If the data is not sized to the size as required by the study, this will
   * resize the data.
   */
  RayData & data(const std::size_t i);
  /**
   * Gets a read only reference to the Ray's data at an index.
   *
   * If the data is not sized to the size as required by the study, this will
   * resize the data.
   */
  const RayData & data(const std::size_t i) const;

  /**
   * Gets a writeable reference to the Ray's auxilary data
   *
   * If the aux data is not sized to the size as required by the study, this will
   * resize the aux data.
   */
  std::vector<RayData> & auxData();
  /**
   * Gets a read only reference to the Ray's auxilary data
   *
   * If the aux data is not sized to the size as required by the study, this will
   * resize the aux data.
   */
  const std::vector<RayData> & auxData() const;
  /**
   * Gets a writeable reference to a component of the Ray's auxilary data
   *
   * If the aux data is not sized to the size as required by the study, this will
   * resize the aux data.
   */
  RayData & auxData(const std::size_t i);
  /**
   * Gets a read only reference to a component of the Ray's auxilary data
   *
   * If the aux data is not sized to the size as required by the study, this will
   * resize the aux data.
   */
  const RayData & auxData(const std::size_t i) const;

  /**
   * Sets the information pretaining to the start point for the Ray.
   *
   * This MUST be called before setStartingDirection(), setStartingEndPoint(),
   * or setStartingMaxDistance(). It cannot be called after a Ray has
   * began tracing.
   *
   * @param starting_point The starting point
   * @param starting_elem The starting element (if known)
   * @param starting_incoming_side The starting incoming side (if known and if
   * the Ray starts on a side of \p starting_elem
   */
  void setStart(const Point & starting_point,
                const Elem * starting_elem = nullptr,
                const unsigned short starting_incoming_side = RayTracingCommon::invalid_side);
  /**
   * Sets the starting direction to \p starting_direction for a Ray.
   *
   * This MUST be called after setStart(). It cannot be used with
   * setStartingEndPoint(), which sets the direction internally.
   * It cannot be called after a Ray has began tracing.
   */
  void setStartingDirection(const Point & starting_direction);
  /**
   * Sets the starting end point to \p starting_point for a Ray.
   *
   * This MUST be called after setStart(). It cannot be used with
   * setStartingDirection(). It cannot be called after a Ray has began tracing.
   *
   * Internally, this sets the direction to be
   * currentPoint() -> \p starting_direction, and sets the maximum
   * distance to || \p starting_direction - currentPoint() ||.
   */
  void setStartingEndPoint(const Point & starting_end_point);
  /**
   * Sets the maximum distance this Ray should travel to \p starting_max_distance.
   *
   * This MUST be called after setStart(). It cannot be used with
   * setStartingEndPoint(). It cannot be called after a Ray has began tracing.
   *
   * If setting a Ray's trajectory with setStartingEndPoint(), the max distance
   * is set internally to be || end - start ||.
   *
   * Can only be called before a Ray has started to be traced!
   */
  void setStartingMaxDistance(const Real starting_max_distance);

  /**
   * Invalidates a Ray's starting element.
   *
   * This is useful after the mesh has changed due to adaptivity,
   * in which the starting element may no longer be valid.
   *
   * This can only be called before a Ray has began tracing.
   */
  void invalidateStartingElem();
  /**
   * Invalidates a Ray's starting incoming side.
   *
   * This is useful after the mesh has changed due to adaptivity,
   * in which the incoming side may no longer be valid.
   *
   * This can only be called before a Ray has began tracing.
   */
  void invalidateStartingIncomingSide();
  /**
   * Clears the starting information set on the Ray:
   * - Starting point
   * - Starting element
   * - Starting incoming side
   * - Starting direction
   * - Starting maximum distance
   *
   * This can only be called before a Ray has began tracing.
   */
  void clearStartingInfo();

  /**
   * Clears the internal counters on the Ray so that the Ray can be traced again.
   *
   * Can only be used within generateRays().
   */
  void resetCounters();

  /**
   * Gets the current element that the Ray is in.
   *
   * Before tracing, this is the starting element for the Ray.
   *
   * During tracing:
   * - It is valid within RayKernels, because a Ray can only operate on a
   *   single element per segment.
   * - When used on boundaries (RayBCs), it is the element that the
   *   Ray actually traced through. When on a boundary, a RayBC may be
   *   applied to multiple elements when hitting a vertex or edge.
   *   Therefore, this will be only one of said elements.
   */
  const Elem * currentElem() const { return _current_elem; }

  /**
   * Get a Ray's current incoming side
   *
   * Before tracing, this is the starting incoming side (if any).
   *
   * During and after tracing, this is ONLY guaranteed to be valid
   * while executing RayKernels!
   */
  unsigned short currentIncomingSide() const { return _current_incoming_side; }
  /**
   * Whether or not the Ray's current incoming side is invalid
   *
   * Before tracing, this is the starting incoming side (if any).
   *
   * During and after tracing, this is ONLY guaranteed to be valid
   * while executing RayKernels!
   */
  bool invalidCurrentIncomingSide() const
  {
    return _current_incoming_side == RayTracingCommon::invalid_side;
  }

  /**
   * Whether or not the user has set an end point for this Ray.
   *
   * This is done by limiting the distance of the Ray in its set direction.
   */
  bool endSet() const { return _end_set; }
  /**
   * Whether or not the Ray is at the user-defined end point.
   *
   * This is only valid when the user set the trajectory of the Ray
   * with setStartingEndPoint(), which internally set its maximum distance
   * to the straight-line distance between start and end and set
   * endSet() == true.
   */
  bool atEnd() const;
  /**
   * Gets the user-set end point for the Ray, if set.
   *
   * Internally, we do not keep track of the end point. Instead,
   * we set a maximum straight-line distance the Ray can travel
   * until it hits its endpoint. With the current point, the distance,
   * the maximum distance, and the direction, we can infer the
   * user-set end point.
   */
  Point endPoint() const;

  /**
   * Gets the number of times this Ray has crossed a processor
   */
  unsigned int processorCrossings() const { return _processor_crossings; }

  /**
   * Gets the number of intersections this Ray has done
   */
  unsigned int intersections() const { return _intersections; }

  /**
   * Gets the distance this Ray has traveled
   */
  Real distance() const { return _distance; }
  /**
   * Gets the max distance this Ray is allowed to travel
   *
   * This may be set internally to || end - start || in the case that
   * the user initialized the Ray with setStartingEndPoint().
   */
  Real maxDistance() const { return _max_distance; }
  /**
   * Whether or not the distance has been set via setStartingMaxDistance()
   */
  bool maxDistanceSet() const { return _max_distance != std::numeric_limits<Real>::max(); }

  /**
   * Whether or not this Ray should continue
   */
  bool shouldContinue() const { return _should_continue; }
  /**
   * Sets whether or not this Ray should continue
   */
  void setShouldContinue(const bool should_continue) { _should_continue = should_continue; }

  /**
   * Whether or not this Ray has had its trajectory changed
   */
  bool trajectoryChanged() const { return _trajectory_changed; }
  /**
   * Gets the number of trajectory changes this Ray has had
   */
  unsigned int trajectoryChanges() const { return _trajectory_changes; }

  /**
   * Helper function for getting information about the Ray
   */
  std::string getInfo() const;

  /**
   * Whether or not a Ray has begun tracing
   */
  bool hasTraced() const
  {
    return (bool)_distance || (bool)_processor_crossings || (bool)_intersections;
  }

  /**
   * Get the RayTracingStudy associated with this Ray
   */
  const RayTracingStudy & study() const { return _study; }

private:
  /**
   * Changes the Ray's ID
   */
  void changeID(const RayID id) { _id = id; }

  /**
   * Invalidates the Ray's current element
   */
  void invalidateCurrentElem() { _current_elem = nullptr; }

  /**
   * Sets the Ray's current point
   */
  void setCurrentPoint(const Point & current_point) { _current_point = current_point; }
  /**
   * Invalidates the Ray's current point
   */
  void invalidateCurrentPoint() { _current_point = RayTracingCommon::invalid_point; }

  /**
   * Sets the Ray's direction
   */
  void setDirection(const Point & direction) { _direction = direction; }
  /**
   * Invalidates the Ray's current direction
   */
  void invalidateDirection() { _direction = RayTracingCommon::invalid_point; }

  /**
   * Change a Ray's current elem
   */
  void setCurrentElem(const Elem * current_elem) { _current_elem = current_elem; }

  /**
   * Change a Ray's incoming side
   */
  void setCurrentIncomingSide(const unsigned short current_incoming_side)
  {
    _current_incoming_side = current_incoming_side;
  }
  /**
   * Invalidates the Ray's current incoming side
   */
  void invalidateCurrentIncomingSide() { _current_incoming_side = RayTracingCommon::invalid_side; }

  /**
   * Set whether or not this Ray has had its trajectory changed
   */
  void setTrajectoryChanged(const bool trajectory_changed)
  {
    _trajectory_changed = trajectory_changed;
  }

  /**
   * Increment the Ray's processor crossing counter
   */
  void addProcessorCrossing() { ++_processor_crossings; }

  /**
   * Increment the Ray's intersection counter
   */
  void addIntersection() { ++_intersections; }

  /**
   * Increment the Ray's trajectory change counter
   */
  void addTrajectoryChange() { ++_trajectory_changes; }

  /**
   * Adds to the distance this Ray has traveled
   */
  void addDistance(const Real add_distance) { _distance += add_distance; }
  /**
   * Changes the Ray's max distance to be traveled
   */
  void changeMaxDistance(const Real max_distance) { _max_distance = max_distance; }
  /**
   * Invalidates the Ray's max distance
   */
  void invalidateMaxDistance() { _max_distance = std::numeric_limits<Real>::max(); }

  /**
   * Produces a useful error if a Ray has started tracing
   */
  void errorIfTracing(const std::string & reason) const;
  /**
   * Produces a useful error for use when initializing a Ray
   */
  void errorWhenInitializing(const std::string & reason) const;

  /**
   * Reset all of the internal counters
   */
  void resetCountersInternal();

  /**
   * Helper for the equality operators.
   */
  bool equalityHelper(const Ray & other, const bool equal) const;

  /**
   * Clears the starting information.
   */
  void clearStartingInfoInternal();

  /// A unique ID for this Ray
  RayID _id;

  /**
   * Current point of the Ray.
   *
   * Before tracing, this is the starting point for the Ray.
   * During tracing, this is the furthest ahead that a Ray has traced. For example,
   * when on a segment in a RayKernel, this will be end of said segment.
   * After tracing, this is where the Ray ended.
   */
  Point _current_point;

  /// Direction of the Ray
  Point _direction;

  /**
   * Current element that the Ray is in.
   *
   * Before tracing, this is the starting element for the Ray.
   *
   * During tracing:
   * - It is valid within RayKernels, because a Ray can only operate on a
   *   single element per segment.
   * - When used on boundaries (RayBCs), it is the element that the
   *   Ray actually traced through. When on a boundary, a RayBC may be
   *   applied to multiple elements when hitting a vertex or edge.
   *   Therefore, this will be only one of said elements.
   */
  const Elem * _current_elem;

  /**
   * The side of _current_elem that the Ray is incoming on (if any).
   *
   * Before tracing, this is the starting incoming side (if any).
   *
   * During tracing, this is ONLY guaranteed to be valid during
   * the execution of RayKernels!
   */
  unsigned short _current_incoming_side;

  /// Whether or not the user has set an end point for this Ray (via limiting its
  /// distance with setStartingEndPoint())
  bool _end_set;

  /// Number of times this Ray has been communicated
  unsigned int _processor_crossings;

  /// Number of intersections done for this Ray
  unsigned int _intersections;

  /// Number of times this Ray has had its trajectory changed
  unsigned int _trajectory_changes;
  /// Whether or not this Ray had its trajectory changed (not sent in parallel)
  bool _trajectory_changed;

  /// Total distance this Ray has traveled
  Real _distance;
  /// Maximum distance the Ray is allowed to travel
  Real _max_distance;

  /// Wether or not the Ray should continue to be traced (not sent in parallel)
  bool _should_continue;

  /// The data that is carried with the Ray
  /// This is mutable so that we can resize it if needed within const accessors
  mutable std::vector<RayData> _data;

  /// Auxiliary data that is carried with the ray
  /// This is mutable so that we can resize it if needed within const accessors
  mutable std::vector<RayData> _aux_data;

  /// The RayTracingStudy that owns this Ray (not sent in parallel)
  RayTracingStudy & _study;

  /// Extra padding to avoid false sharing
  long padding[8];

  // TraceRay is the only object that should be executing Rays and therefore needs access
  friend class TraceRay;
  // Packing needs access to changing the internal counters during the trace
  friend class Parallel::Packing<std::shared_ptr<Ray>>;
  // Allows for testing of equality methods
  friend class TestRayLots;
  // Data helpers needs to be able to access the internal methods for a Ray for store/load
  friend void dataStore(std::ostream & stream, std::shared_ptr<Ray> & ray, void * context);
  friend void dataLoad(std::istream & stream, std::shared_ptr<Ray> & ray, void * context);
};

/**
 * The following methods are specializations for using the Parallel::packed_range_* routines
 * for a vector of Rays
 */
namespace libMesh
{
namespace Parallel
{
template <>
class Packing<std::shared_ptr<Ray>>
{
public:
  typedef Real buffer_type;

  static unsigned int packed_size(typename std::vector<Real>::const_iterator in);
  static unsigned int packable_size(const std::shared_ptr<Ray> & ray, const void *);
  static unsigned int size(const std::size_t data_size, const std::size_t aux_data_size);

  template <typename Iter, typename Context>
  static void pack(const std::shared_ptr<Ray> & object, Iter data_out, const Context * context);

  template <typename BufferIter, typename Context>
  static std::shared_ptr<Ray> unpack(BufferIter in, Context * context);
};

} // namespace Parallel

} // namespace libMesh

void dataStore(std::ostream & stream, std::shared_ptr<Ray> & ray, void * context);
void dataLoad(std::istream & stream, std::shared_ptr<Ray> & ray, void * context);
