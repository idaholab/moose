[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top right bottom left'
  study = study
[]

[UserObjects]
  [study]
    type = TestRayLots
    execute_on = initial

    vertex_to_vertex = true
    centroid_to_vertex = true
    centroid_to_centroid = true
    side_aq = true
    centroid_aq = true

    ray_kernel_coverage_check = false
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
