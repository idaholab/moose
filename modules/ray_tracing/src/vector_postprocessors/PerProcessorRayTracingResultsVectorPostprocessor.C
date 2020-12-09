//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerProcessorRayTracingResultsVectorPostprocessor.h"

// Local includes
#include "RayTracingStudy.h"

#include "TraceRay.h"
#include "ParallelRayStudy.h"

registerMooseObject("RayTracingApp", PerProcessorRayTracingResultsVectorPostprocessor);

InputParameters
PerProcessorRayTracingResultsVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addClassDescription(
      "Accumulates ray tracing results (information about the trace) on a per-processor basis.");

  params.addRequiredParam<UserObjectName>("study", "The RayTracingStudy to get results from");

  const std::string result_names =
      "rays_started rays_traced chunks_traced rays_received buffers_received rays_sent "
      "buffers_sent intersections generation_time propagation_time num_probes ray_pool_created "
      "receive_ray_pool_created receive_buffer_pool_created send_buffer_pool_created face_hit "
      "vertex_hit edge_hit moved_through_neighbors backface_culling_successes "
      "backface_culling_failures intersection_calls vertex_neighbor_builds vertex_neighbor_lookups "
      "edge_neighbor_builds edge_neighbor_lookups point_neighbor_builds failed_traces";
  MultiMooseEnum results(result_names, result_names);

  params.addParam<MultiMooseEnum>("results", results, "The selection of results you want reported");

  return params;
}

PerProcessorRayTracingResultsVectorPostprocessor::PerProcessorRayTracingResultsVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _study(getUserObject<RayTracingStudy>("study")),
    _results(getParam<MultiMooseEnum>("results")),
    _pid(processor_id()),
    _pid_values(declareVector("pid"))
{
  auto num_procs = n_processors();

  _pid_values.resize(num_procs, 0);

  for (auto & result : _results)
  {
    std::string lower_result_name;

    std::transform(result.name().begin(),
                   result.name().end(),
                   std::back_inserter(lower_result_name),
                   ::tolower);

    auto id = result.id();

    _result_values[id] = &declareVector(lower_result_name);

    _result_values[id]->resize(num_procs, 0);
  }
}

void
PerProcessorRayTracingResultsVectorPostprocessor::initialize()
{
}

void
PerProcessorRayTracingResultsVectorPostprocessor::execute()
{
  _pid_values[_pid] = _pid;

  for (auto & result : _results)
  {
    switch (result)
    {
      case 0: // rays_started
        (*_result_values[0])[_pid] = _study.parallelRayStudy().localWorkStarted();
        break;
      case 1: // rays_traced
        (*_result_values[1])[_pid] = _study.parallelRayStudy().localWorkExecuted();
        break;
      case 2: // chunks_traced
        (*_result_values[2])[_pid] = _study.parallelRayStudy().localChunksExecuted();
        break;
      case 3: // rays_received
        (*_result_values[3])[_pid] = _study.parallelRayStudy().receiveBuffer().objectsReceived();
        break;
      case 4: // buffers_received
        (*_result_values[4])[_pid] = _study.parallelRayStudy().receiveBuffer().buffersReceived();
        break;
      case 5: // rays_sent
        (*_result_values[5])[_pid] = _study.parallelRayStudy().parallelDataSent();
        break;
      case 6: // buffers_sent
        (*_result_values[6])[_pid] = _study.parallelRayStudy().buffersSent();
        break;
      case 7: // intersections
        (*_result_values[7])[_pid] = _study.localTraceRayResult(TraceRay::INTERSECTIONS);
        break;
      case 8: // generation_time
        (*_result_values[8])[_pid] = _study.generationTime();
        break;
      case 9: // propagation_time
        (*_result_values[9])[_pid] = _study.propagationTime();
        break;
      case 10: // num_probes
        (*_result_values[10])[_pid] = _study.parallelRayStudy().receiveBuffer().numProbes();
        break;
      case 11: // ray_pool_created
        (*_result_values[11])[_pid] = _study.parallelRayStudy().poolParallelDataCreated();
        break;
      case 12: // receive_ray_pool_created
        (*_result_values[12])[_pid] = _study.parallelRayStudy().receiveBuffer().objectPoolCreated();
        break;
      case 13: // receive_buffer_pool_created
        (*_result_values[13])[_pid] = _study.parallelRayStudy().receiveBuffer().bufferPoolCreated();
        break;
      case 14: // send_buffer_pool_created
        (*_result_values[14])[_pid] = _study.parallelRayStudy().sendBufferPoolCreated();
        break;
      case 15: // face_hit
        (*_result_values[15])[_pid] = _study.localTraceRayResult(TraceRay::FACE_HITS);
        break;
      case 16: // vertex_hit
        (*_result_values[16])[_pid] = _study.localTraceRayResult(TraceRay::VERTEX_HITS);
        break;
      case 17: // edge_hit
        (*_result_values[17])[_pid] = _study.localTraceRayResult(TraceRay::EDGE_HITS);
        break;
      case 18: // moved_through_neighbors
        (*_result_values[18])[_pid] = _study.localTraceRayResult(TraceRay::MOVED_THROUGH_NEIGHBORS);
        break;
      case 19: // backface_culling_successes
        (*_result_values[19])[_pid] =
            _study.localTraceRayResult(TraceRay::BACKFACE_CULLING_SUCCESSES);
        break;
      case 20: // backface_culling_failures
        (*_result_values[20])[_pid] =
            _study.localTraceRayResult(TraceRay::BACKFACE_CULLING_FAILURES);
        break;
      case 21: // intersection_calls
        (*_result_values[21])[_pid] = _study.localTraceRayResult(TraceRay::INTERSECTION_CALLS);
        break;
      case 22: // vertex_neighbor_builds
        (*_result_values[22])[_pid] = _study.localTraceRayResult(TraceRay::VERTEX_NEIGHBOR_BUILDS);
        break;
      case 23: // vertex_neighbor_lookups
        (*_result_values[23])[_pid] = _study.localTraceRayResult(TraceRay::VERTEX_NEIGHBOR_LOOKUPS);
        break;
      case 24: // edge_neighbor_builds
        (*_result_values[24])[_pid] = _study.localTraceRayResult(TraceRay::EDGE_NEIGHBOR_BUILDS);
        break;
      case 25: // edge_neighbor_lookups
        (*_result_values[25])[_pid] = _study.localTraceRayResult(TraceRay::EDGE_NEIGHBOR_LOOKUPS);
        break;
      case 26: // point_neighbor_builds
        (*_result_values[26])[_pid] = _study.localTraceRayResult(TraceRay::POINT_NEIGHBOR_BUILDS);
        break;
      case 27: // failed_traces
        (*_result_values[27])[_pid] = _study.localTraceRayResult(TraceRay::FAILED_TRACES);
        break;
      default:
        mooseError("Unknown result type '",
                   result.name(),
                   "/",
                   result.id(),
                   "' in PerProcessorRayTracingResultsVectorPostprocessor ",
                   name());
    }
  }
}

void
PerProcessorRayTracingResultsVectorPostprocessor::finalize()
{
  // TODO: Should be able to just "gather" to proc zero - but that's not working....
  gatherMax(_pid_values);

  for (auto & result : _results)
    gatherMax(*_result_values[result.id()]);
}
