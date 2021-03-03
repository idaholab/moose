#include "ParallelRayStudy.h"

// Local includes
#include "TraceRay.h"
#include "RayTracingStudy.h"

ParallelRayStudy::ParallelRayStudy(
    RayTracingStudy & ray_tracing_study,
    const std::vector<std::shared_ptr<TraceRay>> & threaded_trace_ray)
  : ParallelStudy<MooseUtils::SharedPool<Ray>::PtrType, Ray>(
        ray_tracing_study.comm(), ray_tracing_study.parameters(), "ParallelRayStudy"),
    _ray_tracing_study(ray_tracing_study),
    _threaded_trace_ray(threaded_trace_ray)
{
}

void
ParallelRayStudy::postExecuteChunk(const work_iterator begin, const work_iterator end)
{
  for (auto it = begin; it != end; ++it)
  {
    auto & ray = *it;

    // The Ray is done tracing
    if (!ray->shouldContinue())
      _ray_tracing_study.onCompleteRay(std::move(ray));
    // Going to another processor
    else
    {
      mooseAssert(ray->currentElem()->processor_id() != _pid,
                  "Continuing Ray not going to another processor");

      moveParallelDataToBuffer(std::move(ray), ray->currentElem()->processor_id());
    }

    mooseAssert(!ray, "Ray was not moved");
  }
}

bool
ParallelRayStudy::workIsComplete(const MooseUtils::SharedPool<Ray>::PtrType & ray)
{
  // "Work" (a Ray) is considered complete in the parallel algorithm when it is done tracing
  return !ray->shouldContinue();
}

void
ParallelRayStudy::postReceiveParallelData(const parallel_data_iterator begin,
                                          const parallel_data_iterator end)
{
  // Move all of the parallel data (Rays that are continuing to be traced on this processor)
  // directly into the work buffer
  moveContinuingWorkToBuffer(begin, end);
}

void
ParallelRayStudy::executeWork(MooseUtils::SharedPool<Ray>::PtrType & ray, const THREAD_ID tid)
{
  mooseAssert(ray->shouldContinue(), "Tracing Ray that should not continue");

  // If this is false, it means we have a Ray that is banked to go onto another processor
  if (ray->currentElem()->processor_id() == _pid)
    _threaded_trace_ray[tid]->trace(*ray);
}
