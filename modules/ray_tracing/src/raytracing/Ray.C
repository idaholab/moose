//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Ray.h"

// Local includes
#include "ParallelRayStudy.h"
#include "RayTracingStudy.h"
#include "RayTracingPackingUtils.h"

// MOOSE includes
#include "DataIO.h"

using namespace libMesh;

Ray::Ray(RayTracingStudy * study,
         const RayID id,
         const std::size_t data_size,
         const std::size_t aux_data_size,
         const bool /* reset */,
         const ConstructRayKey &)
  : _id(id),
    _current_point(RayTracingCommon::invalid_point),
    _direction(RayTracingCommon::invalid_point),
    _current_elem(nullptr),
    _current_incoming_side(RayTracingCommon::invalid_side),
    _end_set(false),
    _max_distance(std::numeric_limits<Real>::max()),
    _data(data_size, 0),
    _aux_data(aux_data_size, 0),
    _study(*study)
{
  resetCountersInternal();
}

void
Ray::reset(RayTracingStudy * libmesh_dbg_var(study),
           const RayID id,
           const std::size_t data_size,
           const std::size_t aux_data_size,
           const bool reset,
           const ConstructRayKey &)
{
  _id = id;
  _data.resize(data_size, 0);
  _aux_data.resize(aux_data_size, 0);

  if (reset)
  {
    resetCountersInternal();
    _current_point = RayTracingCommon::invalid_point;
    _direction = RayTracingCommon::invalid_point;
    _current_elem = nullptr;
    _current_incoming_side = RayTracingCommon::invalid_side;
    _end_set = false;
    _max_distance = std::numeric_limits<Real>::max();
    std::fill(_data.begin(), _data.end(), 0);
    std::fill(_aux_data.begin(), _aux_data.end(), 0);
  }

  mooseAssert(study == &_study, "Resetting Ray from different study");
}

Ray::Ray(const Ray * const other, const ConstructRayKey & key)
  : Ray(&other->_study,
        other->_id,
        other->_data.size(),
        other->_aux_data.size(),
        /* reset = */ true,
        key)
{
  other->errorIfTracing("Cannot copy Ray");

  if (!other->invalidCurrentPoint())
    setStart(other->_current_point, other->_current_elem, other->_current_incoming_side);

  if (other->_end_set)
    setStartingEndPoint(other->endPoint());
  else
  {
    if (!other->invalidDirection())
      setStartingDirection(other->_direction);
    if (other->maxDistanceSet())
      _max_distance = other->_max_distance;
  }

  std::copy(other->_data.begin(), other->_data.end(), _data.begin());
  std::copy(other->_aux_data.begin(), other->_aux_data.end(), _aux_data.begin());
}

void
Ray::reset(const Ray * const other, const ConstructRayKey &)
{
  other->errorIfTracing("Cannot copy Ray");
  mooseAssert(&other->_study == &_study, "Cannot copy Ray from different study");

  clearStartingInfoInternal();
  resetCountersInternal();

  _id = other->_id;

  if (!other->invalidCurrentPoint())
    setStart(other->_current_point, other->_current_elem, other->_current_incoming_side);

  if (other->_end_set)
    setStartingEndPoint(other->endPoint());
  else
  {
    if (!other->invalidDirection())
      setStartingDirection(other->_direction);
    if (other->maxDistanceSet())
      _max_distance = other->_max_distance;
  }

  _data = other->_data;
  _aux_data = other->_aux_data;
}

bool
Ray::equalityHelper(const Ray & other, const bool equal) const
{
  if (this == &other)
    return equal;

  if (_id != other._id)
    return !equal;
  if (invalidCurrentPoint() != other.invalidCurrentPoint() ||
      !_current_point.absolute_fuzzy_equals(other._current_point))
    return !equal;
  if (invalidDirection() != other.invalidDirection() ||
      !_direction.absolute_fuzzy_equals(other._direction))
    return !equal;
  if (_current_elem != other._current_elem)
    return !equal;
  if (_current_incoming_side != other._current_incoming_side)
    return !equal;
  if (_end_set != other._end_set)
    return !equal;
  if (_processor_crossings != other._processor_crossings)
    return !equal;
  if (_intersections != other._intersections)
    return !equal;
  if (_trajectory_changes != other._trajectory_changes)
    return !equal;
  if (_trajectory_changed != other._trajectory_changed)
    return !equal;
  if (!MooseUtils::absoluteFuzzyEqual(_distance, other._distance))
    return !equal;
  if (!MooseUtils::absoluteFuzzyEqual(_max_distance, other._max_distance))
    return !equal;
  if (_should_continue != other._should_continue)
    return !equal;
  if (_data.size() != other._data.size())
    return !equal;
  for (std::size_t i = 0; i < _data.size(); ++i)
    if (!MooseUtils::absoluteFuzzyEqual(_data[i], other._data[i]))
      return !equal;
  if (_aux_data.size() != other._aux_data.size())
    return !equal;
  for (std::size_t i = 0; i < _aux_data.size(); ++i)
    if (!MooseUtils::absoluteFuzzyEqual(_aux_data[i], other._aux_data[i]))
      return !equal;
  if (&_study != &other._study)
    return !equal;

  return equal;
}

bool
Ray::atEnd() const
{
  if (!_end_set)
    mooseError("Cannot use Ray::atEnd() for a Ray that does not have an end set\n\n", getInfo());
  mooseAssert(_distance <= _max_distance + TOLERANCE * TOLERANCE, "Distance past max distance");

  return MooseUtils::absoluteFuzzyEqual(_distance, _max_distance);
}

Point
Ray::endPoint() const
{
  if (!_end_set)
    mooseError("Cannot use Ray::endPoint() for a Ray that does not have an end set\n\n", getInfo());
  mooseAssert(_distance <= _max_distance + TOLERANCE * TOLERANCE, "Distance past max distance");

  return _current_point + (_max_distance - _distance) * _direction;
}

void
Ray::changeDirection(const Point & direction, const ChangeDirectionKey)
{
  if (direction.absolute_fuzzy_equals(Point(0, 0, 0)))
    mooseError("Cannot set zero vector direction for a Ray\n\n", getInfo());

  _direction = direction.unit();
  _trajectory_changed = true;
}

void
Ray::changeStartDirection(const Point & start,
                          const Point & direction,
                          const ChangeStartDirectionKey)
{
  if (direction.absolute_fuzzy_equals(Point(0, 0, 0)))
    mooseError("Cannot set zero vector direction for a Ray\n\n", getInfo());

  _current_point = start;
  _direction = direction.unit();
  _trajectory_changed = true;
}

void
Ray::setStart(const Point & starting_point,
              const Elem * starting_elem /* = nullptr */,
              const unsigned short starting_incoming_side /* = RayTracingCommon::invalid_side */)
{
  mooseAssert(starting_point != RayTracingCommon::invalid_point, "Invalid point");
  errorIfTracing("Cannot use Ray::setStart()");
  if (!invalidCurrentPoint() && !_current_point.absolute_fuzzy_equals(starting_point))
    errorWhenInitializing("Starting point was already set via Ray::setStart() and is being changed."
                          "\n\nYou may only call Ray::setStart() after it has been called once to"
                          "\nchange the starting element and starting incoming side."
                          "\n\nYou may also clear the starting info via Ray::clearStartingInfo().");

  _current_point = starting_point;
  _current_elem = starting_elem;
  _current_incoming_side = starting_incoming_side;

  if (_study.verifyRays())
  {
    if (!_study.looseBoundingBox().contains_point(starting_point))
      errorWhenInitializing("Mesh does not contain starting point.");
    if (starting_elem)
    {
      mooseAssert(_study.meshBase().query_elem_ptr(starting_elem->id()) == starting_elem,
                  "Element is not owned by the mesh");
      if (!starting_elem->active())
        errorWhenInitializing("Starting element is not active.");
    }

    bool non_planar_start = false;

    if (!invalidCurrentIncomingSide())
    {
      if (!starting_elem)
        errorWhenInitializing("Starting incoming side is set but starting element is not set.");
      if (starting_elem->n_sides() < starting_incoming_side)
        errorWhenInitializing("Starting incoming side is not valid for its starting element.");

      non_planar_start = _study.sideIsNonPlanar(starting_elem, starting_incoming_side);
      if (!non_planar_start &&
          !starting_elem->build_side_ptr(starting_incoming_side)->contains_point(starting_point))
        errorWhenInitializing("Starting incoming side does not contain the starting point.");
    }

    if (starting_elem && !non_planar_start && !starting_elem->contains_point(starting_point))
      errorWhenInitializing("Starting element does not contain the starting point.");
  }
}

void
Ray::setStartingDirection(const Point & starting_direction)
{
  errorIfTracing("Cannot use Ray::setStartingDirection()");
  if (invalidCurrentPoint())
    errorWhenInitializing("Cannot use Ray::setStartingDirection() before Ray::setStart().");
  if (!invalidDirection())
    errorWhenInitializing(
        "Cannot change a Ray's starting direction using Ray::setStartingDirection()"
        "\nafter it has already been set."
        "\n\nYou must first clear the starting info using Ray::clearStartingInfo().");
  if (starting_direction.absolute_fuzzy_equals(Point(0, 0, 0)))
    errorWhenInitializing("Starting direction in Ray::setStartingDirection() is the zero vector.");

  _direction = starting_direction.unit();
}

void
Ray::setStartingEndPoint(const Point & starting_end_point)
{
  errorIfTracing("Cannot use Ray::setStartingEndPoint()");
  if (invalidCurrentPoint())
    errorWhenInitializing("Cannot use Ray::setStartingEndPoint() before Ray::setStart().");
  if (_current_point.absolute_fuzzy_equals(starting_end_point))
    errorWhenInitializing("End point is equal to the start point in Ray::setStartingEndPoint().");
  if (!invalidDirection())
    errorWhenInitializing("Cannot use Ray::setStartingEndPoint() after Ray::setStartingDirection()."
                          "\n\nClear the starting information with Ray::clearStartingInfo().");
  if (maxDistanceSet())
    errorWhenInitializing(
        "Cannot use Ray::setStartingEndPoint() after Ray::setStartingMaxDistance().");

  if (_study.verifyRays() && !_study.looseBoundingBox().contains_point(starting_end_point))
    errorWhenInitializing("End point is not within the mesh for Ray::setStartingEndPoint().");

  Point difference = starting_end_point;
  difference -= _current_point;
  setStartingMaxDistance(difference.norm());
  setStartingDirection(difference);
  _end_set = true;
}

void
Ray::setStartingMaxDistance(const Real starting_max_distance)
{
  errorIfTracing("Cannot use Ray::setStartingMaxDistance()");
  if (invalidCurrentPoint())
    errorWhenInitializing("Cannot use Ray::setStartingMaxDistance() before Ray::setStart().");
  if (starting_max_distance <= 0)
    errorWhenInitializing("Starting max distance is <= 0 in Ray::setStartingMaxDistance().");
  if (_end_set)
    errorWhenInitializing(
        "Cannot use Ray::setStartingMaxDistance() after Ray::setStartingEndPoint().");

  _max_distance = starting_max_distance;
}

void
Ray::setStationary()
{
  errorIfTracing("Cannot use Ray::setStationary()");
  if (invalidCurrentPoint())
    errorWhenInitializing("Cannot use Ray::setStationary() before Ray::setStart()");
  if (_end_set)
    errorWhenInitializing("Cannot use Ray::setStationary() after Ray::setStartingEndPoint()");
  if (!invalidDirection())
    errorWhenInitializing("Cannot use Ray::setStationary() with Ray::setStartingDirection()");
  _max_distance = 0;
  mooseAssert(stationary(), "Stationary not set");
}

void
Ray::invalidateStartingElem()
{
  errorIfTracing("Cannot use Ray::invalidateStartingElem()");
  invalidateCurrentElem();
}

void
Ray::invalidateStartingIncomingSide()
{
  errorIfTracing("Cannot use Ray::invalidateStartingIncomingSide()");
  invalidateCurrentIncomingSide();
}

void
Ray::clearStartingInfo()
{
  errorIfTracing("Cannot use Ray::clearStartingInfo()");
  clearStartingInfoInternal();
}

void
Ray::clearStartingInfoInternal()
{
  invalidateCurrentPoint();
  invalidateCurrentElem();
  invalidateCurrentIncomingSide();
  invalidateDirection();
  invalidateMaxDistance();
  _end_set = false;
}

void
Ray::errorIfTracing(const std::string & reason) const
{
  if (hasTraced())
    mooseError(reason, " after it has started tracing\n\n", getInfo());
}

void
Ray::errorWhenInitializing(const std::string & reason) const
{
  mooseError("While initializing starting information for a Ray:\n\n", reason, "\n\n", getInfo());
}

void
Ray::resetCounters()
{
  if (!_study.currentlyGenerating())
    mooseError("Ray::resetCounters() can only be used during generateRays()\n\n", getInfo());
  resetCountersInternal();
}

void
Ray::resetCountersInternal()
{
  _processor_crossings = 0;
  _intersections = 0;
  _trajectory_changes = 0;
  _distance = 0;
  _trajectory_changed = false;
  _should_continue = true;
}

std::vector<RayData> &
Ray::data()
{
  mooseAssert(_data.size() == 0 || _data.size() == _study.rayDataSize(),
              "Ray data size of " + std::to_string(_data.size()) +
                  " is not zero or the size required by the study of " +
                  std::to_string(_study.rayDataSize()));
  _data.resize(_study.rayDataSize());
  return _data;
}

const std::vector<RayData> &
Ray::data() const
{
  mooseAssert(_data.size() == 0 || _data.size() == _study.rayDataSize(),
              "Ray data size is not zero or the size required by study");
  _data.resize(_study.rayDataSize());
  return _data;
}

RayData &
Ray::data(const std::size_t i)
{
  mooseAssert(_study.rayDataSize() > i, "Accessing Ray data out of range");
  return data()[i];
}

const RayData &
Ray::data(const std::size_t i) const
{
  mooseAssert(_study.rayDataSize() > i, "Accessing Ray data out of range");
  return data()[i];
}

std::vector<RayData> &
Ray::auxData()
{
  mooseAssert(_aux_data.size() == 0 || _aux_data.size() == _study.rayAuxDataSize(),
              "Ray data size is not zero or the size required by study");
  _aux_data.resize(_study.rayAuxDataSize());
  return _aux_data;
}

const std::vector<RayData> &
Ray::auxData() const
{
  mooseAssert(_aux_data.size() == 0 || _aux_data.size() == _study.rayAuxDataSize(),
              "Ray data size is not zero or the size required by study");
  _aux_data.resize(_study.rayAuxDataSize());
  return _aux_data;
}

RayData &
Ray::auxData(const std::size_t i)
{
  mooseAssert(_study.rayAuxDataSize() > i, "Accessing Ray data out of range");
  return auxData()[i];
}

const RayData &
Ray::auxData(const std::size_t i) const
{
  mooseAssert(_study.rayAuxDataSize() > i, "Accessing Ray data out of range");
  return auxData()[i];
}

std::string
Ray::getInfo() const
{
  std::ostringstream oss;

  oss << "Ray information with " << _study.type() << " '" << _study.name() << "' on pid "
      << _study.comm().rank() << "\n";
  oss << "  this = " << this << "\n";
  oss << "  id() = " << id() << "\n";
  if (_study.useRayRegistration() && !invalidID())
    oss << "  _study.registeredRayName(id()) = " << _study.registeredRayName(id()) << "\n";
  oss << "  currentPoint() = ";
  if (invalidCurrentPoint())
    oss << "invalid point\n";
  else
    oss << currentPoint() << "\n";
  oss << "  direction() = ";
  if (invalidDirection())
    oss << "invalid point\n";
  else
    oss << direction() << "\n";
  oss << "  currentIncomingSide() = ";
  if (invalidCurrentIncomingSide())
    oss << "invalid side\n";
  else
    oss << currentIncomingSide() << "\n";
  oss << "  endSet() = " << (endSet() ? "true" : "false") << "\n";
  if (endSet())
  {
    oss << "  endPoint() = " << endPoint() << "\n";
    oss << "  atEnd() = " << (atEnd() ? "true" : "false") << "\n";
  }
  oss << "  distance() = " << distance() << "\n";
  oss << "  maxDistance() = " << maxDistance() << "\n";
  if (currentElem())
  {
    oss << "  currentElem()->id() = " << currentElem()->id() << "\n";
    oss << "  currentElem()->processor_id() = " << currentElem()->processor_id() << "\n";
  }
  else
    oss << "  currentElem()->id() = invalid\n";
  oss << "  processorCrossings() = " << processorCrossings() << "\n";
  oss << "  intersections() = " << intersections() << "\n";
  oss << "  trajectoryChanges() = " << trajectoryChanges() << "\n";
  oss << "  shouldContinue() = " << (shouldContinue() ? "true" : "false") << "\n";
  oss << "  trajectoryChanged() = " << (trajectoryChanged() ? "true" : "false") << "\n";
  oss << "  data() = ";
  for (std::size_t i = 0; i < data().size(); ++i)
    oss << "\n    '" << _study.getRayDataName(i) << "' = " << data(i);
  oss << "\n";
  oss << "  auxData() = ";
  for (std::size_t i = 0; i < auxData().size(); ++i)
    oss << "\n    '" << _study.getRayAuxDataName(i) << "' = " << auxData(i);
  oss << "\n";

  return oss.str();
}

namespace libMesh
{
namespace Parallel
{

unsigned int
Packing<std::shared_ptr<Ray>>::size(const std::size_t data_size, const std::size_t aux_data_size)
{
  // Current incoming side, end_set, processor crossings, intersections, trajectory
  // changes (packed into as few buffer_type as possible: 5 values stored as 2 Reals)
  constexpr unsigned int mixed_size = RayTracingPackingUtils::
      mixedPackSize<buffer_type, unsigned short, bool, unsigned int, unsigned int, unsigned int>();
  mooseAssert(mixed_size == 2, "Mixed size should be 2");

  // First value: size of data, size of aux data, id, current point (3 values), direction (3
  // values), current element, distance, max distance
  // Second value: mixed size (see above)
  auto size = 12 + mixed_size;

#ifdef SINGLE_PRECISION_RAY
  if (data_size)
    size += RayTracingPackingUtils::reinterpretCopySize<RayData, buffer_type>(data_size);
  if (aux_data_size)
    size += RayTracingPackingUtils::reinterpretCopySize<RayData, buffer_type>(aux_data_size);
#else
  size += data_size + aux_data_size;
#endif

  return size;
}

unsigned int
Packing<std::shared_ptr<Ray>>::packed_size(typename std::vector<buffer_type>::const_iterator in)
{
  const std::size_t data_size = static_cast<std::size_t>(*in++);
  const std::size_t aux_data_size = static_cast<std::size_t>(*in);

  return size(data_size, aux_data_size);
}

unsigned int
Packing<std::shared_ptr<Ray>>::packable_size(const std::shared_ptr<Ray> & ray, const void *)
{
  return size(ray->data().size(), ray->auxData().size());
}

template <>
std::shared_ptr<Ray>
Packing<std::shared_ptr<Ray>>::unpack(std::vector<buffer_type>::const_iterator in,
                                      ParallelStudy<std::shared_ptr<Ray>, Ray> * study)
{
  mooseAssert(dynamic_cast<ParallelRayStudy *>(study), "Not a ParallelRayStudy");
  RayTracingStudy & ray_tracing_study = static_cast<ParallelRayStudy *>(study)->rayTracingStudy();

  // Grab the data size
  const std::size_t data_size = static_cast<std::size_t>(*in++);
  const std::size_t aux_data_size = static_cast<std::size_t>(*in++);

  // ID
  RayID id;
  RayTracingPackingUtils::unpack(*in++, id);

  std::shared_ptr<Ray> ray =
      ray_tracing_study.acquireRayInternal(id,
                                           data_size,
                                           aux_data_size,
                                           /* reset = */ false,
                                           RayTracingStudy::AcquireRayInternalKey());

  // Current Point
  ray->_current_point(0) = *in++;
  ray->_current_point(1) = *in++;
  ray->_current_point(2) = *in++;

  // Direction
  ray->_direction(0) = *in++;
  ray->_direction(1) = *in++;
  ray->_direction(2) = *in++;

  // Current Element
  RayTracingPackingUtils::unpack(ray->_current_elem, *in++, &ray_tracing_study.meshBase());

  // Current incoming size, end set, processor crossings, intersections, trajectory changes
  // (unpacked from as few buffer_type as possible - 5 values from 2 Reals)
  RayTracingPackingUtils::mixedUnpack<buffer_type>(in,
                                                   ray->_current_incoming_side,
                                                   ray->_end_set,
                                                   ray->_processor_crossings,
                                                   ray->_intersections,
                                                   ray->_trajectory_changes);

  // Distance
  ray->_distance = *in++;

  // Max distance
  ray->_max_distance = *in++;

#ifdef SINGLE_PRECISION_RAY
  RayTracingPackingUtils::reinterpretUnpackCopy<buffer_type>(ray->_data, in);
  RayTracingPackingUtils::reinterpretUnpackCopy<buffer_type>(ray->_aux_data, in);
#else
  // Copy out data
  RayTracingPackingUtils::unpackCopy(ray->_data, in);
  RayTracingPackingUtils::unpackCopy(ray->_aux_data, in);
#endif

  ray->_should_continue = true;
  ray->_trajectory_changed = false;

  return ray;
}

template <>
void
Packing<std::shared_ptr<Ray>>::pack(const std::shared_ptr<Ray> & ray,
                                    std::back_insert_iterator<std::vector<buffer_type>> data_out,
                                    const ParallelStudy<std::shared_ptr<Ray>, Ray> * study)
{
  mooseAssert(dynamic_cast<const ParallelRayStudy *>(study), "Not a ParallelRayStudy");
  const RayTracingStudy & ray_tracing_study =
      static_cast<const ParallelRayStudy *>(study)->rayTracingStudy();
  mooseAssert(&ray->study() == &ray_tracing_study, "Packing Ray for different study");

  // Storing the data size first makes it easy to verify and reserve space
  data_out = static_cast<buffer_type>(ray->_data.size());
  data_out = static_cast<buffer_type>(ray->_aux_data.size());

  // ID
  data_out = RayTracingPackingUtils::pack<buffer_type>(ray->id());

  // Current Point
  data_out = ray->_current_point(0);
  data_out = ray->_current_point(1);
  data_out = ray->_current_point(2);

  // Direction
  data_out = ray->_direction(0);
  data_out = ray->_direction(1);
  data_out = ray->_direction(2);

  // Current element
  data_out =
      RayTracingPackingUtils::pack<buffer_type>(ray->_current_elem, &ray_tracing_study.meshBase());

  // Current incoming size, end set, processor crossings, intersections, trajectory changes
  // (packed into as few buffer_type as possible - 5 values into 2 Reals
  RayTracingPackingUtils::mixedPack<buffer_type>(data_out,
                                                 ray->_current_incoming_side,
                                                 ray->_end_set,
                                                 ray->_processor_crossings,
                                                 ray->_intersections,
                                                 ray->_trajectory_changes);

  // Distance
  data_out = ray->_distance;
  // Max distance
  data_out = ray->_max_distance;

  // Copy out data
#ifdef SINGLE_PRECISION_RAY
  RayTracingPackingUtils::reinterpretPackCopy<buffer_type>(ray->_data, data_out);
  RayTracingPackingUtils::reinterpretPackCopy<buffer_type>(ray->_aux_data, data_out);
#else
  std::copy(ray->_data.begin(), ray->_data.end(), data_out);
  std::copy(ray->_aux_data.begin(), ray->_aux_data.end(), data_out);
#endif
}

} // namespace Parallel

} // namespace libMesh

void
dataStore(std::ostream & stream, std::shared_ptr<Ray> & ray, void * context)
{
  mooseAssert(ray, "Null ray");
  mooseAssert(context, "Missing RayTracingStudy context");
  mooseAssert(static_cast<RayTracingStudy *>(context) == &ray->study(), "Different study");

  storeHelper(stream, ray->_id, context);
  storeHelper(stream, ray->_current_point, context);
  storeHelper(stream, ray->_direction, context);
  auto current_elem_id = ray->currentElem() ? ray->currentElem()->id() : DofObject::invalid_id;
  storeHelper(stream, current_elem_id, context);
  storeHelper(stream, ray->_current_incoming_side, context);
  storeHelper(stream, ray->_end_set, context);
  storeHelper(stream, ray->_processor_crossings, context);
  storeHelper(stream, ray->_intersections, context);
  storeHelper(stream, ray->_trajectory_changes, context);
  storeHelper(stream, ray->_trajectory_changed, context);
  storeHelper(stream, ray->_distance, context);
  storeHelper(stream, ray->_max_distance, context);
  storeHelper(stream, ray->_should_continue, context);
  storeHelper(stream, ray->_data, context);
  storeHelper(stream, ray->_aux_data, context);
}

void
dataLoad(std::istream & stream, std::shared_ptr<Ray> & ray, void * context)
{
  mooseAssert(context, "Missing RayTracingStudy context");
  RayTracingStudy * study = static_cast<RayTracingStudy *>(context);

  RayID id;
  loadHelper(stream, id, context);
  ray = study->acquireRayInternal(id,
                                  /* data_size = */ 0,
                                  /* aux_data_size = */ 0,
                                  /* reset = */ true,
                                  RayTracingStudy::AcquireRayInternalKey());

  loadHelper(stream, ray->_current_point, context);
  loadHelper(stream, ray->_direction, context);
  dof_id_type current_elem_id;
  loadHelper(stream, current_elem_id, context);
  ray->_current_elem = study->meshBase().query_elem_ptr(current_elem_id);
  loadHelper(stream, ray->_current_incoming_side, context);
  loadHelper(stream, ray->_end_set, context);
  loadHelper(stream, ray->_processor_crossings, context);
  loadHelper(stream, ray->_intersections, context);
  loadHelper(stream, ray->_trajectory_changes, context);
  loadHelper(stream, ray->_trajectory_changed, context);
  loadHelper(stream, ray->_distance, context);
  loadHelper(stream, ray->_max_distance, context);
  loadHelper(stream, ray->_should_continue, context);
  loadHelper(stream, ray->_data, context);
  loadHelper(stream, ray->_aux_data, context);

  if (ray->hasTraced())
    mooseAssert(!study->currentlyGenerating() && !study->currentlyPropagating(),
                "Cannot not load a Ray that has already traced during generation or propagation; "
                "reset the Ray first");
}
