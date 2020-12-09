#pragma once

#include "ParallelStudy.h"

// Local includes
#include "Ray.h"

// Forward declarations
class RayTracingStudy;
class TraceRay;

class ParallelRayStudy : public ParallelStudy<std::shared_ptr<Ray>, Ray, RayTracingStudy>
{
public:
  ParallelRayStudy(RayTracingStudy & study,
                   const std::vector<std::shared_ptr<TraceRay>> & threaded_trace_ray);

protected:
  void executeWork(const std::shared_ptr<Ray> & ray, const THREAD_ID tid) override;
  void moveWorkError(const MoveWorkError error, const std::shared_ptr<Ray> * ray) const override;
  void postReceiveParallelData(const parallel_data_iterator begin,
                               const parallel_data_iterator end) override;
  bool workIsComplete(const std::shared_ptr<Ray> & ray) override;
  void postExecuteChunk(const work_iterator begin, const work_iterator end) override;

  /// The RayTracingStudy
  RayTracingStudy & _ray_tracing_study;
  /// The TraceRay objects that do the tracing for each thread
  const std::vector<std::shared_ptr<TraceRay>> & _threaded_trace_ray;
};
