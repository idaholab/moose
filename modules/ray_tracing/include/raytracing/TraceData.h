#pragma once

// Local includes
#include "Ray.h"

// MOOSE includes
#include "MooseError.h"

// libMesh includes
#include "libmesh/point.h"

/**
 * Data structure that stores the necessary information for outputting a Ray at a point
 */
struct TracePointData
{
  TracePointData(const libMesh::Point & point) : _point(point)
  {
    mooseAssert(_point != RayTracingCommon::invalid_point, "Invalid point");
  }

  /// The point on _elem this segment leaves from
  libMesh::Point _point;
  /// The data on the Ray after this segment is traced (optional)
  std::vector<RayData> _data;
  /// The aux data on the Ray after this segment is traced (optional)
  std::vector<RayData> _aux_data;
};

/**
 * Data structure that stores information for output of a partial trace of a Ray on a processor
 */
struct TraceData
{
  TraceData(const std::shared_ptr<Ray> & ray)
    : _ray_id(ray->id()),
      _intersections(ray->intersections()),
      _processor_crossings(ray->processorCrossings()),
      _trajectory_changes(ray->trajectoryChanges()),
      _last(false)
  {
    mooseAssert(_ray_id != Ray::INVALID_RAY_ID, "Invalid Ray ID");
    addPoint(ray->currentPoint());
  }

  void addPoint(const libMesh::Point & point) { _point_data.emplace_back(point); }

  TracePointData & lastPoint() { return _point_data.back(); }

  unsigned int numSegments() const
  {
    mooseAssert(!_point_data.empty(), "Should not be empty");
    return _point_data.size() - 1;
  }

  /// The Ray ID
  const RayID _ray_id;
  /// The number of intersections thus far
  const unsigned long int _intersections;
  /// Number of processor crossings thus far
  const unsigned int _processor_crossings;
  /// Number of trajectory changes thus far
  const unsigned int _trajectory_changes;
  /// Whether or not this was the last set of segments for this Ray
  bool _last;
  /// The data for each point along the track
  std::vector<TracePointData> _point_data;
};
