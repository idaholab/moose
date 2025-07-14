[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 3
  nx = 3
  ny = 3
  nz = 3
[]

[RayBCs/periodic]
  type = PeriodicRayBC
  auto_direction = 'x y z'
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = true
  edge_to_edge = true
  execute_on = initial
  ray_distance = 3
[]

[Postprocessors/total_distance]
  type = RayTracingStudyResult
  study = lots
  result = total_distance
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
[]
