# PerProcessorRayTracingResultsVectorPostprocessor

!syntax description /VectorPostprocessors/PerProcessorRayTracingResultsVectorPostprocessor

This object is particularly useful for quantifying load balance between processors when
using the [ray_tracing/index.md]. It pairs well with a [VectorPostprocessorVisualizationAux.md]
for visualization.

The non-advanced results that are available are as follows:

- `rays_started`: The number of rays started on a processor
- `rays_finished`: The number of rays finished (rays that end) on a processor
- `rays_received`: The number of rays received on a processor
- `rays_sent`: The number of rays sent from a processor
- `intersections`: The number of intersections encountered by all rays on a processor
- `generation_time`: The time spent in `generateRays()` on a processor
- `propagation_time`: The time spent in `propagateRays()` on a processor
- `ray_pool_created`: The number of rays created in the [RayTracingStudy.md#ray-pool] on a processor
- `face_hit`: The number of trace hits on faces on a processor
- `vertex_hit`: The number of trace hits on vertices on a processor
- `edge_hit`: The number of trace hits on edges on a processor
- `moved_through_neighbors`: The number of times a trace moves through neighbors on a processor
- `intersection_calls`: The number of times a trace has attempted to find an intersection on an element, on a processor
- `vertex_neighbor_builds`: The number of times the neighbor map for a vertex has been built on a processor
- `vertex_neighbor_lookups`: The number of times the neighbor map for a vertex has been looked up on a processor
- `edge_neighbor_builds`: The number of times the neighbor map for an edge has been built on a processor
- `edge_neighbor_lookups`: The number of times the neighbor map for an edge has been looked up on a processor
- `point_neighbor_builds`: The number of times a point neighbor lookup has been performed on a processor
- `failed_traces`: The number of allowed trace failures that have occurred on a processor

!syntax parameters /VectorPostprocessors/PerProcessorRayTracingResultsVectorPostprocessor

!syntax inputs /VectorPostprocessors/PerProcessorRayTracingResultsVectorPostprocessor

!syntax children /VectorPostprocessors/PerProcessorRayTracingResultsVectorPostprocessor
