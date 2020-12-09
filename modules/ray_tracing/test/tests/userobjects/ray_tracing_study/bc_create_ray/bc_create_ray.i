[Mesh]
  active = gmg_2d
  [gmg_2d]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmax = 3
    ymax = 3
  []
  [gmg_3d]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmax = 3
    ymax = 3
    zmax = 3
  []
[]

[RayBCs]
  active = 'kill_2d create_2d'

  [kill_2d]
    type = KillRayBC
    boundary = 'top right bottom left'
  []
  [create_2d]
    type = CreateRayRayBCTest
    boundary = 'top right bottom left'
  []

  [kill_3d]
    type = KillRayBC
    boundary = 'top right bottom left front back'
  []
  [create_3d]
    type = CreateRayRayBCTest
    boundary = 'top right bottom left front back'
  []
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  execute_on = initial

  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = true

  ray_kernel_coverage_check = false
[]

[Postprocessors]
  [total_distance]
    type = RayTracingStudyResult
    study = lots
    result = total_distance
  []
  [total_rays_started]
    type = RayTracingStudyResult
    study = lots
    result = total_rays_started
  []
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
