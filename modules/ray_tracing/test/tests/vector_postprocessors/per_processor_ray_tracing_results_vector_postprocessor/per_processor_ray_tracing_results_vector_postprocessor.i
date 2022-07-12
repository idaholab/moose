[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 2
    xmax = 5
    ymax = 2
  []
[]

[AuxVariables]
  [rays_started]
    family = MONOMIAL
    order = CONSTANT
  []
  [rays_traced]
    family = MONOMIAL
    order = CONSTANT
  []
  [rays_received]
    family = MONOMIAL
    order = CONSTANT
  []
  [rays_sent]
    family = MONOMIAL
    order = CONSTANT
  []
  [intersections]
    family = MONOMIAL
    order = CONSTANT
  []
  [face_hit]
    family = MONOMIAL
    order = CONSTANT
  []
  [vertex_hit]
    family = MONOMIAL
    order = CONSTANT
  []
  [edge_hit]
    family = MONOMIAL
    order = CONSTANT
  []
  [moved_through_neighbors]
    family = MONOMIAL
    order = CONSTANT
  []
  [intersection_calls]
    family = MONOMIAL
    order = CONSTANT
  []
  [vertex_neighbor_lookups]
    family = MONOMIAL
    order = CONSTANT
  []
  [edge_neighbor_lookups]
    family = MONOMIAL
    order = CONSTANT
  []
  [point_neighbor_builds]
    family = MONOMIAL
    order = CONSTANT
  []
[]

# these results are not output because they cannot be golded against
# but we still make sure they work:
#
# generation_time propagation_time chunks_traced
# buffers_received buffers_sent ray_pool_created
# receive_ray_pool_created receive_buffer_pool_created

[AuxKernels]
  [rays_started]
    type = VectorPostprocessorVisualizationAux
    variable = rays_started
    vpp = per_proc_ray_tracing
    vector_name = rays_started
    execute_on = timestep_end
  []
  [rays_traced]
    type = VectorPostprocessorVisualizationAux
    variable = rays_traced
    vpp = per_proc_ray_tracing
    vector_name = rays_traced
    execute_on = timestep_end
  []
  [rays_received]
    type = VectorPostprocessorVisualizationAux
    variable = rays_received
    vpp = per_proc_ray_tracing
    vector_name = rays_received
    execute_on = timestep_end
  []
  [rays_sent]
    type = VectorPostprocessorVisualizationAux
    variable = rays_sent
    vpp = per_proc_ray_tracing
    vector_name = rays_sent
    execute_on = timestep_end
  []
  [intersections]
    type = VectorPostprocessorVisualizationAux
    variable = intersections
    vpp = per_proc_ray_tracing
    vector_name = intersections
    execute_on = timestep_end
  []
  [face_hit]
    type = VectorPostprocessorVisualizationAux
    variable = face_hit
    vpp = per_proc_ray_tracing
    vector_name = face_hit
    execute_on = timestep_end
  []
  [vertex_hit]
    type = VectorPostprocessorVisualizationAux
    variable = vertex_hit
    vpp = per_proc_ray_tracing
    vector_name = vertex_hit
    execute_on = timestep_end
  []
  [edge_hit]
    type = VectorPostprocessorVisualizationAux
    variable = edge_hit
    vpp = per_proc_ray_tracing
    vector_name = edge_hit
    execute_on = timestep_end
  []
  [moved_through_neighbors]
    type = VectorPostprocessorVisualizationAux
    variable = moved_through_neighbors
    vpp = per_proc_ray_tracing
    vector_name = moved_through_neighbors
    execute_on = timestep_end
  []
  [intersection_calls]
    type = VectorPostprocessorVisualizationAux
    variable = intersection_calls
    vpp = per_proc_ray_tracing
    vector_name = intersection_calls
    execute_on = timestep_end
  []
  [vertex_neighbor_lookups]
    type = VectorPostprocessorVisualizationAux
    variable = vertex_neighbor_lookups
    vpp = per_proc_ray_tracing
    vector_name = vertex_neighbor_lookups
    execute_on = timestep_end
  []
  [edge_neighbor_lookups]
    type = VectorPostprocessorVisualizationAux
    variable = edge_neighbor_lookups
    vpp = per_proc_ray_tracing
    vector_name = edge_neighbor_lookups
    execute_on = timestep_end
  []
  [point_neighbor_builds]
    type = VectorPostprocessorVisualizationAux
    variable = point_neighbor_builds
    vpp = per_proc_ray_tracing
    vector_name = point_neighbor_builds
    execute_on = timestep_end
  []
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false # no need for RayKernels
  execute_on = initial
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top left right bottom'
[]

[VectorPostprocessors/per_proc_ray_tracing]
  type = PerProcessorRayTracingResultsVectorPostprocessor
  execute_on = timestep_end
  study = lots
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
