//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelStudy.h"

// Local includes
#include "Ray.h"

// Forward declarations
class RayTracingStudy;
class TraceRay;

class ParallelRayStudy : public ParallelStudy<std::shared_ptr<Ray>, Ray>
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
