#pragma once

#include "ParallelStudy.h"

// Local includes
#include "Ray.h"

// Forward declarations
class RayTracingStudy;
class TraceRay;

class ParallelRayStudy : public ParallelStudy<MooseUtils::SharedPool<Ray>::PtrType, Ray>
{
public:
  ParallelRayStudy(RayTracingStudy & study,
                   const std::vector<std::shared_ptr<TraceRay>> & threaded_trace_ray);

  /**
   * Get the RayTracingStudy associated with this ParallelRayStudy.
   */
  ///@{
  RayTracingStudy & rayTracingStudy() { return _ray_tracing_study; }
  const RayTracingStudy & rayTracingStudy() const { return _ray_tracing_study; }
  ///@}

protected:
  void executeWork(MooseUtils::SharedPool<Ray>::PtrType & ray, const THREAD_ID tid) override;
  void postReceiveParallelData(const parallel_data_iterator begin,
                               const parallel_data_iterator end) override;
  bool workIsComplete(const MooseUtils::SharedPool<Ray>::PtrType & ray) override;
  void postExecuteChunk(const work_iterator begin, const work_iterator end) override;

  /// The RayTracingStudy
  RayTracingStudy & _ray_tracing_study;
  /// The TraceRay objects that do the tracing for each thread
  const std::vector<std::shared_ptr<TraceRay>> & _threaded_trace_ray;
};
