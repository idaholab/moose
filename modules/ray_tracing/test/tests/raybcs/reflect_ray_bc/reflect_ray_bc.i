[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

[RayBCs]
  [kill]
    type = KillRayBC
    boundary = 'top'
  []
  [reflect]
    type = ReflectRayBC
    boundary = 'top right left bottom'
  []
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = false
  execute_on = initial
  ray_distance = 10
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
  exodus = false
  csv = true
[]
